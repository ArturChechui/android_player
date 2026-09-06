package com.example.player

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import com.example.player.databinding.ActivityMainBinding

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding
    private var currentVolume = 0.8f
    private var isMuted = false
    private var currentStationIndex = 0

    private val stations = arrayOf(
        "http://online.kissfm.ua/KissFM",
        "http://online.kissfm.ua/KissFM_Ukr"
    )

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        nativeInit()

        // Volume Up
        binding.btnVolUp.setOnClickListener {
            currentVolume = (currentVolume + 0.1f).coerceAtMost(1.0f)
            nativeSetVolume(currentVolume)
        }

        // Volume Down
        binding.btnVolDown.setOnClickListener {
            currentVolume = (currentVolume - 0.1f).coerceAtLeast(0.0f)
            nativeSetVolume(currentVolume)
        }

        // Mute Toggle
        binding.btnMute.setOnClickListener {
            isMuted = !isMuted
            nativeSetMute(isMuted)
        }

        // Station Switch
        binding.btnSwitchStation.setOnClickListener {
            currentStationIndex = (currentStationIndex + 1) % stations.size
            nativeSwitchStream(stations[currentStationIndex])
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        nativeDestroy()
    }

    // Native JNI functions
    private external fun nativeInit()
    private external fun nativeSetVolume(volume: Float)
    private external fun nativeSetMute(muted: Boolean)
    private external fun nativeSwitchStream(url: String)
    private external fun nativeDestroy()

    companion object {
        init {
            System.loadLibrary("player")
        }
    }
}