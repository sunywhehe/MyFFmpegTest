package com.leosun.myffmpegtest;

import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.View;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;

import com.hjq.permissions.OnPermissionCallback;
import com.hjq.permissions.Permission;
import com.hjq.permissions.XXPermissions;

import java.io.File;
import java.util.List;

public class MainActivity extends AppCompatActivity {

    private static final String TAG = "ffmpeg_lib";

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("ffmpeg_lib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Example of a call to a native method
        TextView tv = findViewById(R.id.sample_text);
        tv.setText(getFFmpegVersion());

        findViewById(R.id.decode_btn).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                requestPermission();
            }
        });
    }

    private void requestPermission() {
        XXPermissions.with(this)
                .permission(Permission.MANAGE_EXTERNAL_STORAGE)
                .request(new OnPermissionCallback() {

                    @Override
                    public void onGranted(List<String> permissions, boolean all) {
                        if (all) {
                            createConfigFile();
                        } else {

                        }
                    }

                    @Override
                    public void onDenied(List<String> permissions, boolean never) {
                        if (never) {
                            // 如果是被永久拒绝就跳转到应用权限系统设置页面
                            XXPermissions.startPermissionActivity(MainActivity.this, permissions);
                        } else {
                        }
                    }
                });
    }

    private void createConfigFile() {
        File rootFile = new File(Environment.getExternalStorageDirectory() + "/test/ffmpeg");
        Log.d(TAG, "root file: " + rootFile.getAbsolutePath());
        if (!rootFile.exists()) {
            rootFile.mkdirs();
        } else {
            Log.d(TAG, "createConfigFile root exists");
        }
        File mp4file = new File(rootFile, "marver.mp4");
        File yuvfile = new File(rootFile, "marver.yuv");
        new Thread() {
            @Override
            public void run() {
                videoDecode(mp4file.getAbsolutePath(), yuvfile.getAbsolutePath());
            }
        }.start();
    }


    /**
     * @return 返回当前
     */
    public native static String getFFmpegVersion();

    public native static void videoDecode(String inputStr, String outputStr);
}