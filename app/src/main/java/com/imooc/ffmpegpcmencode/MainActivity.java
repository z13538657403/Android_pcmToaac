package com.imooc.ffmpegpcmencode;

import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;

public class MainActivity extends AppCompatActivity
{
    static
    {
        System.loadLibrary("avutil-54");
        System.loadLibrary("avcodec-56");
        System.loadLibrary("avformat-56");
        System.loadLibrary("avdevice-56");
        System.loadLibrary("swresample-1");
        System.loadLibrary("swscale-3");
        System.loadLibrary("postproc-53");
        System.loadLibrary("avfilter-5");
        System.loadLibrary("sffhelloworld");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        String path = Environment.getExternalStorageDirectory().getAbsolutePath()
                + "/record/encode.pcm";
        int result = pcmToaac(path , Environment.getExternalStorageDirectory().getAbsolutePath() + "/transfer.aac");
    }

    public native String helloFFmpeg();
    public native int pcmToaac(String srcPath , String audioPath);
}
