package com.libcmpc;

import android.app.Activity;
import android.os.Bundle;
import android.widget.TextView;

public class MainActivity extends Activity {
    static {
        System.loadLibrary("cmpc");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        TextView view = new TextView(this);
        view.setText("LibCMPC packaging stub");
        setContentView(view);
    }
}
