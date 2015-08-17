package org.cocos2dx.lib;

import android.content.Context;
import android.util.Log;

import com.google.vrtoolkit.cardboard.sensors.HeadTracker;

import com.google.atap.tangoservice.Tango;
import com.google.atap.tangoservice.Tango.OnTangoUpdateListener;
import com.google.atap.tangoservice.TangoConfig;
import com.google.atap.tangoservice.TangoCoordinateFramePair;
import com.google.atap.tangoservice.TangoErrorException;
import com.google.atap.tangoservice.TangoEvent;
import com.google.atap.tangoservice.TangoOutOfDateException;
import com.google.atap.tangoservice.TangoPoseData;
import com.google.atap.tangoservice.TangoXyzIjData;
/**
 * Created by huabingxu on 8/13/15.
 */
public class Cocos2dxDeviceTracker
{
    private HeadTracker _headTracker = null;
    boolean _isHeadTracking = false;

    Cocos2dxDeviceTracker()
    {
        Context activity = Cocos2dxActivity.getContext();

        _headTracker = HeadTracker.createFromContext(activity);

    };

    public void startTracking()
    {
        if(!_isHeadTracking)
        {
            _headTracker.startTracking();
            _isHeadTracking = true;
        }

    }

    public void stopTracking()
    {
        if(_isHeadTracking)
        {
            _headTracker.stopTracking();
            _isHeadTracking = false;
        }

    }

    public float[] getHeadTracking()
    {
        float [] result = new float[16];
        _headTracker.getLastHeadView(result,0);

        return result;
    }

}
