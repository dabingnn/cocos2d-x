/****************************************************************************
Copyright (c) 2010-2011 cocos2d-x.org

http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
 ****************************************************************************/
package org.cocos2dx.lib;

import android.content.Context;
import android.content.res.Configuration;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.util.Log;
import android.view.Display;
import android.view.Surface;
import android.view.WindowManager;
import android.os.Build.*;

public class Cocos2dxGyroscope implements SensorEventListener {
    // ===========================================================
    // Constants
    // ===========================================================

    private static final String TAG = Cocos2dxGyroscope.class.getSimpleName();

    // ===========================================================
    // Fields
    // ===========================================================

    private final Context mContext;
    private final SensorManager mSensorManager;
    private final Sensor mGyroscope;
    private final int mNaturalOrientation;
    private boolean mLastTimeStampSet;
    private long mLastTimeStamp;
    // ===========================================================
    // Constructors
    // ===========================================================

    public Cocos2dxGyroscope(final Context context) {
        this.mContext = context;
        this.mLastTimeStamp = 0;
        this.mLastTimeStampSet = false;
        this.mSensorManager = (SensorManager) this.mContext.getSystemService(Context.SENSOR_SERVICE);
        this.mGyroscope = this.mSensorManager.getDefaultSensor(Sensor.TYPE_GYROSCOPE);
        final Display display = ((WindowManager) this.mContext.getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay();
        this.mNaturalOrientation = display.getOrientation();
    }

    // ===========================================================
    // Getter & Setter
    // ===========================================================

    public void enable() {
        this.mSensorManager.registerListener(this, this.mGyroscope, SensorManager.SENSOR_DELAY_GAME);
    }

        public void setInterval(float interval) {
            // Honeycomb version is 11
            if(android.os.Build.VERSION.SDK_INT < 11) {
            this.mSensorManager.registerListener(this, this.mGyroscope, SensorManager.SENSOR_DELAY_GAME);
        } else {
            //convert seconds to microseconds
            this.mSensorManager.registerListener(this, this.mGyroscope, (int)(interval*100000));
        }
    }
      
    public void disable() {
        this.mSensorManager.unregisterListener(this);
    }

    // ===========================================================
    // Methods for/from SuperClass/Interfaces
    // ===========================================================

    @Override
    public void onSensorChanged(final SensorEvent sensorEvent) {
        final int type = sensorEvent.sensor.getType();
        if (type != Sensor.TYPE_GYROSCOPE)
        {
            return;
        }
        if(mLastTimeStampSet == false)
        {
            mLastTimeStampSet = true;
            mLastTimeStamp = sensorEvent.timestamp;
        }
        float x = sensorEvent.values[0];
        float y = sensorEvent.values[1];
        final float z = sensorEvent.values[2];

        // /*
        //  * Because the axes are not swapped when the device's screen orientation
        //  * changes. So we should swap it here. In tablets such as Motorola Xoom,
        //  * the default orientation is landscape, so should consider this.
        //  */
        final int orientation = this.mContext.getResources().getConfiguration().orientation;

        if ((orientation == Configuration.ORIENTATION_LANDSCAPE) && (this.mNaturalOrientation != Surface.ROTATION_0)) {
            final float tmp = x;
            x = -y;
            y = tmp;
        } else if ((orientation == Configuration.ORIENTATION_PORTRAIT) && (this.mNaturalOrientation != Surface.ROTATION_0)) {
            final float tmp = x;
            x = y;
            y = -tmp;
        }       
        
        Cocos2dxGLSurfaceView.queueGyroscope(x,y,z,sensorEvent.timestamp - mLastTimeStamp);
        mLastTimeStamp = sensorEvent.timestamp;
        /*
        if(BuildConfig.DEBUG) {
            Log.d(TAG, "x = " + sensorEvent.values[0] + " y = " + sensorEvent.values[1] + " z = " + pSensorEvent.values[2]);
        }
        */
    }

    @Override
    public void onAccuracyChanged(final Sensor sensor, final int accuracy) {
    }

    // ===========================================================
    // Methods
        // Native method called from Cocos2dxGLSurfaceView (To be in the same thread)
    // ===========================================================
    
    public static native void onSensorChanged(final float x, final float y, final float z, final long deltaTime);

    // ===========================================================
    // Inner and Anonymous Classes
    // ===========================================================
}
