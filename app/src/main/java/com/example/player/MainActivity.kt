package com.example.player

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import com.example.player.databinding.ActivityMainBinding

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        binding.sampleText.text = "Playing Low-Latency 440Hz Sine Wave..."
        startAudio()
    }

    override fun onDestroy() {
        super.onDestroy()
        stopAudio()
    }

    external fun startAudio()
    external fun stopAudio()

    companion object {
        init {
            System.loadLibrary("player")
        }
    }
}