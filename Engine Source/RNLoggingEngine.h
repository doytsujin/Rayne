//
//  RNLoggingEngine.h
//  Rayne
//
//  Copyright 2013 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef __RAYNE_LOGGINGENGINE_H__
#define __RAYNE_LOGGINGENGINE_H__

#include "RNBase.h"
#include "RNLogging.h"
#include "RNObject.h"

namespace RN
{
	namespace Log
	{
		class LoggingEngine : public Object
		{
		public:
			virtual void Open() = 0;
			virtual void Close() = 0;
			virtual bool IsOpen() const = 0;
			
			virtual void CutOff() = 0;
			virtual void Write(const Message& message) = 0;
			
			RNDefineMeta(LoggingEngine, Object)
		};
		
		class StreamLoggingInternal;
		class StdoutLoggingEngine : public LoggingEngine, public Singleton<StdoutLoggingEngine>
		{
		public:
			StdoutLoggingEngine();
			
			virtual void Open() final;
			virtual void Close() final;
			virtual bool IsOpen() const final;
			
			virtual void CutOff() final;
			virtual void Write(const Message& message) final;
			
		private:
			PIMPL<StreamLoggingInternal> _internal;
			
			RNDefineMeta(StdoutLoggingEngine, LoggingEngine)
		};
		
		class SimpleLoggingEngine : public LoggingEngine, public Singleton<SimpleLoggingEngine>
		{
		public:
			SimpleLoggingEngine();
			
			virtual void Open() final;
			virtual void Close() final;
			virtual bool IsOpen() const final;
			
			virtual void CutOff() final;
			virtual void Write(const Message& message) final;
			
		private:
			std::fstream _stream;
			PIMPL<StreamLoggingInternal> _internal;
			
			RNDefineMeta(SimpleLoggingEngine, LoggingEngine)
		};
		
		class HTMLLoggingEngine : public LoggingEngine, public Singleton<HTMLLoggingEngine>
		{
		public:
			virtual void Open() final;
			virtual void Close() final;
			virtual bool IsOpen() const final;
			
			virtual void CutOff() final;
			virtual void Write(const Message& message) final;
			
		private:
			void WriteCSSBoilerplate();
			void SwitchMode(int mode);
			
			std::fstream _stream;
			int _mode;
			
			RNDefineMeta(HTMLLoggingEngine, LoggingEngine)
		};
	}
}

#endif /* __RAYNE_LOGGINGENGINE_H__ */