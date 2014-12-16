﻿//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"
#include <string>

using namespace BlinkyCpp;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::Devices::Enumeration;
using namespace Windows::Devices::Gpio;
using namespace concurrency;

MainPage::MainPage()
{
	InitializeComponent();

	InitGPIO();

	timer_ = ref new DispatcherTimer();
	TimeSpan interval;
	interval.Duration = 500 * 1000 * 10;
	timer_->Interval = interval;
	timer_->Tick += ref new EventHandler<Object ^>(this, &MainPage::OnTick);
	timer_->Start();
}

void MainPage::InitGPIO()
{
    auto selector = GpioController::GetDeviceSelector("GPIO_S5");
    create_task(DeviceInformation::FindAllAsync(selector, nullptr)).then([this](task<DeviceInformationCollection ^> collectionOp) {
        try
        {
            auto deviceInfos = collectionOp.get();
            auto deviceId = deviceInfos->GetAt(0)->Id;

            create_task(GpioController::FromIdAsync(deviceId)).then([this](task<GpioController^> controllerOp) {
                try
                {
                    auto controller = controllerOp.get();
                    auto pinInfo = controller->Pins->Lookup(0);
                    pinInfo->TryOpenOutput(GpioPinValue::Low, GpioSharingMode::Exclusive, &outPin_);
                    GpioStatus->Text = "GPIO pin initialized correctly.";
                }
                catch (Exception ^)
                {
                    // TODO (alecont): we need to marshal this back...
                    GpioStatus->Text = "There were problems initializing the GPIO pin.";
                }
            });

        }
        catch (Exception ^)
        {
            // TODO (alecont): we need to marshal this back...
            GpioStatus->Text = "There were problems initializing the GPIO pin.";
        }
    });
}

void MainPage::FlipLED()
{
	if (LEDStatus_ == 0)
	{
		LEDStatus_ = 1;
		if (outPin_ != nullptr)
		{
			outPin_->Value = GpioPinValue::High;
		}
		LED->Fill = redBrush_;
	}
	else
	{
		LEDStatus_ = 0;
		if (outPin_ != nullptr)
		{
			outPin_->Value = GpioPinValue::Low;
		}
		LED->Fill = grayBrush_;
	}
}

void MainPage::TurnOffLED()
{
	if (LEDStatus_ == 1)
	{
		FlipLED();
	}
}

void MainPage::OnTick(Object ^sender, Object ^args)
{
	FlipLED();
}


void MainPage::Delay_ValueChanged(Object^ sender, RangeBaseValueChangedEventArgs^ e)
{
	if (timer_ == nullptr)
	{
		return;
	}
	if (e->NewValue == Delay->Minimum)
	{
		DelayText->Text = "Stopped";
		timer_->Stop();
		TurnOffLED();
	}
	else
	{
		long delay = static_cast<long>(e->NewValue);
		auto txt = std::to_wstring(delay) + L"ms";
		DelayText->Text = ref new String(txt.c_str());
		TimeSpan interval;
		interval.Duration = delay * 1000 * 10;
		timer_->Interval = interval;
		timer_->Start();
	}

}