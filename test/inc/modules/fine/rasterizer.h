#pragma once

#include <inc/includes.h>

#include <base/Module.h>
#include <kernels/base.cl.h>
#include <kernels/fine_rasterizer.cl.h>
#include <base/Buffer.h>
#include <rendering/Render.h>
#include <pipeline/BinRasterizer.h>
#include <pipeline/FineRasterizer.h>
#include "fine_utils.h"

using namespace nr;
using namespace nr::detail;
using namespace testing;

TEST(Fine, Rasterizer)
{
	cl_status err = CL_SUCCESS;

	const nr_uint dim = 5;
	const nr_uint point_count = dim + 1;

	const ScreenDimension screenDim = { 16, 16 };
	const nr_uint totalScreenSize = screenDim.width * screenDim.height;

	const BinQueueConfig config = { 8, 8, 10 };
	const nr_uint binCountX = ceil(((nr_float)screenDim.width) / config.binWidth);
	const nr_uint binCountY = ceil(((nr_float)screenDim.height) / config.binHeight);
	const nr_uint totalBinCount = binCountX * binCountY;
	const nr_uint totalWorkGroupCount = 1;
	const nr_uint totalBinQueuesSize = totalWorkGroupCount * totalBinCount * (config.queueSize + 1);

	const nr_float defaultDepth = 1;
	const nr_float expectedDepth = 0.5f;

	const nr_uint triangleCount = 3;
	std::unique_ptr<Triangle<dim>[]> h_triangles(new Triangle<dim>[triangleCount]);
	std::unique_ptr<nr_uint[]> h_binQueues(new nr_uint[totalBinQueuesSize]);

	std::unique_ptr<RawColorRGBA[]> h_colorBuffer(new RawColorRGBA[totalScreenSize]{});
	std::unique_ptr<nr_float[]> h_depthBuffer(new nr_float[totalScreenSize]{ defaultDepth });

	std::unique_ptr<RawColorRGBA[]> expectedColorBuffer(new RawColorRGBA[totalScreenSize]{});
	std::unique_ptr<nr_float[]> expectedDepthBuffer(new nr_float[totalScreenSize]{ defaultDepth });
	
	tesselateScreen<dim>(screenDim, config, totalWorkGroupCount, expectedDepth, triangleCount, h_triangles.get(), h_binQueues.get(), expectedColorBuffer.get(), expectedDepthBuffer.get());

	auto code = mkFineModule(dim, &err);
	ASSERT_SUCCESS(err);

	auto testee = FineRasterizer(code, &err);
	ASSERT_SUCCESS(err);

	auto q = defaultCommandQueue;

	FrameBuffer frame;
	frame.color = Buffer::make<nr::RawColorRGBA>(defaultContext, CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR, totalScreenSize, h_colorBuffer.get(), &err);
	ASSERT_SUCCESS(err);
	frame.depth = Buffer::make<nr_float>(defaultContext, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, totalScreenSize, h_depthBuffer.get(), &err);
	ASSERT_SUCCESS(err);

	auto d_triangles = Buffer::make<nr_float>(defaultContext, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, triangleCount * sizeof(Triangle<dim>) / sizeof(nr_float), (nr_float*)h_triangles.get(), &err);
	ASSERT_SUCCESS(err);
	auto d_binQueues = Buffer::make<nr_uint>(defaultContext, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, totalBinQueuesSize, h_binQueues.get(), &err);
	ASSERT_SUCCESS(err);

	testee.triangleData = d_triangles;
	testee.dim = screenDim;
	testee.binQueueConfig = config;
	testee.binQueues = d_binQueues;
	testee.workGroupCount = totalWorkGroupCount;
	testee.frameBuffer = frame;

	NDRange<2> global = { binCountX, binCountY };
	NDRange<2> local = { binCountX, binCountY / totalWorkGroupCount };

	ASSERT_SUCCESS(testee.load());
	ASSERT_SUCCESS(q.enqueueKernelCommand<2>(testee, global, local));
	ASSERT_SUCCESS(q.enqueueBufferReadCommand(frame.color, false, totalScreenSize, h_colorBuffer.get()));
	ASSERT_SUCCESS(q.enqueueBufferReadCommand(frame.depth, false, totalScreenSize, h_depthBuffer.get()));
	ASSERT_SUCCESS(q.await());
	ASSERT_TRUE(validateBuffer("color", screenDim.width, screenDim.height, expectedColorBuffer.get(), h_colorBuffer.get()));
	ASSERT_TRUE(validateBuffer("depth", screenDim.width, screenDim.height, expectedDepthBuffer.get(), h_depthBuffer.get()));
}
