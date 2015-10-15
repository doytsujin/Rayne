//
//  __Bootstrap.h
//  Rayne Unit Tests
//
//  Copyright 2015 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef _RAYNE_UNITTESTS_BOOTSTRAP_H_
#define _RAYNE_UNITTESTS_BOOTSTRAP_H_

#include <gtest/gtest.h>
#include <Rayne.h>

namespace RN
{
	extern Kernel *__BootstrapKernel(Application *app, const ArgumentParser &arguments);
	extern void __TearDownKernel(Kernel *kernel);
}

namespace __Bootstrap
{
	class Application : public RN::Application
	{

	};
}

class KernelFixture : public ::testing::Test
{
public:
	KernelFixture() :
		KernelFixture({ "--headless" })
	{}

	KernelFixture(const std::vector<const char *> &arguments) :
		Test(),
		_arguments(arguments.size(), const_cast<const char **>(arguments.data()))
	{}

protected:
	void SetUp() override
	{
		_app = new __Bootstrap::Application();
		_kernel = RN::__BootstrapKernel(_app, _arguments);
	}
	void TearDown() override
	{
		__TearDownKernel(_kernel);
		delete _app;
	}

private:
	__Bootstrap::Application *_app;
	RN::ArgumentParser _arguments;
	RN::Kernel *_kernel;
};


#endif /* _RAYNE_UNITTESTS_BOOTSTRAP_H_ */
