// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/quic/quic_alarm.h"

#include "base/logging.h"

namespace net {

//QuicEpollConnectionHelper::CreateAlarm()->QuicEpollAlarm()构造函数中调用
QuicAlarm::QuicAlarm(Delegate* delegate)
    : delegate_(delegate),
      deadline_(QuicTime::Zero()) {
}

QuicAlarm::~QuicAlarm() {}

//retransmission_alarm_->Set  QuicConnection::SetTimeoutAlarm  QuicAlarm::Update 等都会调用该接口
void QuicAlarm::Set(QuicTime deadline) {
  DCHECK(!IsSet());
  DCHECK(deadline.IsInitialized());
  deadline_ = deadline;
  SetImpl();
}

void QuicAlarm::Cancel() {
  deadline_ = QuicTime::Zero();
  CancelImpl();
}

void QuicAlarm::Update(QuicTime deadline, QuicTime::Delta granularity) {
  if (!deadline.IsInitialized()) {
    Cancel();
    return;
  }
  if (std::abs(deadline.Subtract(deadline_).ToMicroseconds()) <
          granularity.ToMicroseconds()) {
    return;
  }
  Cancel();
  Set(deadline);
  VLOG(4) << "Update  alarm";
}

bool QuicAlarm::IsSet() const {
  return deadline_.IsInitialized();
}

//执行对应AckAlarm  RetransmissionAlarm  SendAlarm  SendAlarm  TimeoutAlarm  PingAlarm  FecAlarm的OnAlarm
void QuicAlarm::Fire() {
  if (!deadline_.IsInitialized()) {
    return;
  }

  deadline_ = QuicTime::Zero();
  QuicTime deadline = delegate_->OnAlarm();
  // delegate_->OnAlarm() might call Set(), in which case  deadline_ will
  // already contain the new value, so don't overwrite it.
  if (!deadline_.IsInitialized() && deadline.IsInitialized()) {
    Set(deadline); //继续设置alarm
	VLOG(4) << "Fire  alarm";
  }
}

}  // namespace net
