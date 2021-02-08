# Internet of Plants - Embedded Software

**Important: Read the Build bulletpoint carefully, it has a few quirks because of bugs upstream**

Firmware for Internet of Plants embedded system. Made for ESP8266.

It connects to an already existent wifi network, and through it connects to a hardcoded [host](https://github.com/internet-of-plants/server), authenticating itself. It provides air humidity, air temperature, soil temperature and soil humidity measurements to the server every few minutes.

Eventually the goal is to use this data, to generate reports and properly analyze what is going on and allow one day for a good automation system. But we don't believe a naive system without much data to be better than simple timers, that's why we don't consider automation before providing a system to build a significant dataset. This is a plant monitor for now. If you want to automate your best bet for now is to buy a power plug with a timer.

There also are plans to use this to track entire grow rooms instead of a single plant. But they are not a current priority.

If there is no wifi credential or iop credentials stored, a captive portal will be open. The device will open its own wifi network, the password is hardcoded (this is a security vuln, we should generate it at compile time and save it externally, like a sticker, but we lack the infra to do it right now).

After connecting to the captive portal you must provide your wifi and iop credentials. If the wifi credential is correct it will login to the wifi network and then authenticate with the iop server.

No server credential is stored in the device, it will use the credentials to generate an access token for that device that is then stored (and can be revoked).

When the device is online and authenticated to the server a new plant is automatically created on your account if it doesn't already exist. Now you are able to track your plant with the iop server. There you also will be able to provide updates to each specific device (or some of them, or all of them). And monitor panics + logs for those devices.

The updates are automatic and happen over the air. Eventually the updates will demand signed binaries.

If some irrecoverable error happens the device will halt and constantly try to update it's binary. All errors are reported to the network (if available). So you must react to it and provide an update to fix it. Since our system is very small and event driven an irrecoverable error won't be solved by restarting, since the error will just happen again almost immediately. Ideally panics shouldn't happen, so easy recovery doesn't seem like a priority, but as the project evolves we can study a panic that restarts the system instead of halting + trying to update (or in addition to that).

If there is no network available it will just halt forever and must be restarted/updated physically.

The project has been designed and tested for the ESP8266-12F board (nodemcu), but it should be easily portable (and eventually part of our goal, but it's far away, right now we only commit to supporting ESP8266).

Datasheet: https://www.electrodragon.com/w/ESP-12F_ESP8266_Wifi_Board

# Dependencies

PlatformIO (nodemcu2 + arduino framework)

Needs OpenSSL in PATH, clang-format and clang-tidy (install LLVM to get it). Be nice to our codebase :).

- https://wiki.openssl.org/index.php/Binaries
- https://releases.llvm.org/download.html

On linux you have to change permissions to be able to deploy to a serial port

```
curl -fsSL https://raw.githubusercontent.com/platformio/platformio-core/master/scripts/99-platformio-udev.rules | sudo tee /etc/udev/rules.d/99-platformio-udev.rules
sudo service udev restart
sudo usermod -a -G dialout $USER
sudo usermod -a -G plugdev $USER
```

# Build

You can just deploy the code to the nodemcu using PlatformIO. Choose `env:release` to deploy to production. After the first deploy you can use the update server.

You must apply the following patches to the local `esp8266/Arduino`. This is a necessary inconvenience while upstream is at `2.7.4` version. They froze version `2`, and `3` is a breaking change. So we live in a unsupported grey area for now. It works and we could make a fork, but this is a highly temporary situation.

The `String` problem is that if a `String` is moved twice it causes a nullptr-deref, and we do that do signify ownership passing. The other thing is that in `3.0` you can inherit from `CertStoreBase` and make your own `CertStore`. Until now people have been hijacking the header guard (which we did), and it's a horrible approach.

**Replace** `Arduino/cores/esp8266/WString.h` **for** [WString.h](https://github.com/esp8266/Arduino/blob/master/cores/esp8266/WString.h)

**Replace** `Arduino/cores/esp8266/WString.cpp` **for** [WString.cpp](https://github.com/esp8266/Arduino/blob/master/cores/esp8266/WString.cpp)

**Replace** `Arduino/libraries/ESP8266WiFi/src/CertStoreBearSSL.h` **for** [CertStoreBearSSL.h](https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WiFi/src/CertStoreBearSSL.h)

**Replace** `Arduino/libraries/ESP8266WiFi/src/CertStoreBearSSL.cpp` **for** [CertStoreBearSSL.cpp](https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WiFi/src/CertStoreBearSSL.cpp)

We are sorry for this inconvenience, but until upstream updates it's the best approach.

# TODO

Grep for "TODO"'s to find the known missing pieces

# Decisions

There have been a few possibly questionable decisions we took through this code. This was thinking about safety, to prevent programmer screw-ups from causing big problems. Performance isn't our bottleneck, since we don't do much in the embedded system, so safety becomes the prime issue. We want a stable system that can run without intervention for long times.

Ideally we would use zero-runtime-cost abstractions, but cpp doesn't help us make them UB proof, so we have to add runtime overhead to deal with it in a way that allows us to sleep at night.

Most decisions are listed here. If you find some other questionable decision please file an issue so we can explain them. Or even make a PR documenting them, and why they were made.

- Using too many static variables/too much heap allocation

    We use a lot of static variables to store long-living objects (or very heavy ones), that are important to the core. Like ESP servers and clients. That allows us to have only one instance ever, and avoid stack-overflows or allocation failures to store those heavy objects that are very important. It also helps us determining a rough number for the minimum memory needed for the entire toolset.

    We also avoid storing big things in the stack because the stack is super small. It's easier to heap allocate and deal with the allocation failure, than to have a stack-overflow reseting the board.

    The only problem those heap allocations may cause is heap fragmentation, we have taken great care to avoid it, but we have been thinking about buffer re-usage and how to remove allocations. Also making them early, so we aren't get by surprise by the lack of memory. Everything big in static memory means a simple static analyzer can catch memory problems.

    This is a big problem in multi-core systems, as the statics have to actually be thread-locals (or when using preemptive/time-slicing schedulers). So be ware if trying to port it to ESP32 (or to ESP8266 FreeRTOS SDK).
    
    We tend to allocate most big things that can't be static, instead of putting it on the stack. Always using smart-pointers (std::unique_ptr, std::shared_ptr, FixedString<SIZE>, Storage<SIZE>...).

- Avoiding moves

    Since cpp doesn't have destructive moves, it can leave our code in an invalid state. Either with a nulled `std::{unique,shared}_ptr`, or with an empty `Result<T, E>`, for example. And since those abstractions are heavily used throughout the code we don't want a human mistake to cause UB, panic or raise exceptions. Even a wrongly moved-out `Option<T>` can cause logical errors.

    To avoid that we try not to move out, getting references when we can. For example using `UNWRAP(_OK,_ERR)_{REF,MUT}`. But you have to be careful to make sure that the reference doesn't outlive its storage (as always). Instead of `UNWRAP_{OK,ERR}`, that move out, although they can be very useful, like moving-out to return the inner value.

    We also hijack moves when we can, to make them operate like copies. For example using the `TYPED_STORAGE(SIZE)`, `Storage<SIZE>` and `FixedString<SIZE>` types. They all wrap a `std::shared_ptr` and moving actually copies, a very cheap copy.

    For `Result<T, E>` you should not use its methods, but only the macros defined in `result.hpp`, like `UNWRAP_{OK,ERR}(_REF,_MUT)`, `IS_{OK,ERR}`, `RESULT_{OK,ERR}`, etc. All methods have a macro alternative, that checks for an emptied Result and properly reports the invalid access location. This makes impossible to debugs errors caused by moved-out-from values, into easily debuggable ones.

- No exceptions, but we halt using the `panic_` macro

    Most errors should be propagated with `Result<T, E>` or even an `Option<T>`. Exceptions should not happen. And critical errors, that can't be recovered, working as the last stand between us and UB should panic with the `panic_(F("Explanation of what went wrong..."))` macro.

    We don't like exceptions. We don't want to pay the runtime price for them (they are disabled by default). We don't use them for regular control flow, or any control flow at all. But we do want a way to fail hard. So we have a function to panic in a way integrated with the platform. It doesn't use any compiler/esp8266 panic internals, so maybe panic is a bad name. But it basically logs the critical error, reports it through the network if it can (to the server, but we could report to nearby devices that we own too, so they can help). And it keeps asking the server for updates.

    We have future plans to improve this, but we should never panic. Panics should be a way to avoid UB when everything went wrong, and quickly fix when detected.

    The embedded code is fairly small and a panic is probably going to be recurrent if no updates happen, so for now halting and allowing external updates to fix it seems the way to go. All errors are reported to the network, if available.

    Panics before having network access + authentication with the central server have a very small code surface to cause. They should be very rare, hopefully impossible. But they are critical bugs if they happens. In this case the device will halt until manually restarted, and they will only be debuggable manually. Those panics are theoretically possible because we have no way to statically garantee their branches are unreachable, but they should be.

    Some software and hardware exceptions may still happen, we don't handle them, but it's a TODO. Panics also still don't support stack-dumps, but it's planned.

- Redundant runtime checks

    There are a few situations where we check for the same runtime problem multiple times. In some places that happens to improve logging. So we can know exactly where the problem happened.
    
    It also happens a lot with `Result<T, E>` and `Option<T>`, because we have to check their state (with `.isSome()`, `IS_OK(res)`, ...) and then unwrap them to move out the inner value (which also checks, panicking if the wanted value is not there).
    
    That's because cpp forces our hand by not having proper sum types with proper pattern matching. The mainstream solutions is just borrowing a pointer, that is null if not available. And force the check onto the user (at the cost of UB if the user makes a mistake), or they check like we do and throw an exception or just plainly cause UB if you try to get a value that's not there. Since UB is considered unnaccetable by us we check by default, panicking in case of a problem. Theoretically we could make zero-runtime-overhead `unsafe_` prefixed alternatives to signal the programmer took extra care, but you would have to convince the community that it will cause significant performance improvements. And that this improvement is important. Since performance is hardly this project's bottleneck: we wish you good luck (or fork).

    `std::variant` uses `get_if` returning `nullptr` if empty, we could mimick its api with something like

    `if (auto *value = result.get_if_ok())`

    It's fairly ergonomic but if used outside of an if, because of developer mistake, we are in for trouble. Therefore we prefer the extra checks (that sometimes the compiler can optimize out), than risking developer mistake causing big problems. So we don't provide an API like this.