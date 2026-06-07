#pragma once

// Init the CST816 over I²C and register the LVGL input device.
// Returns true if the controller responded.
bool touchBegin();

// Consumes a tap event (press-then-release within < 400ms, no drag).
// Returns true exactly once per tap. Use this to cycle views.
bool touchConsumeTap();
