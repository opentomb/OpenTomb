#ifndef BACKENDS_COREAUDIO_H
#define BACKENDS_COREAUDIO_H

#include "backends/base.h"

struct CoreAudioBackendFactory final : public BackendFactory {
public:
    bool init() override;

    bool querySupport(BackendType type) override;

    void probe(DevProbe type, std::string *outnames) override;

    BackendPtr createBackend(ALCdevice *device, BackendType type) override;

    static BackendFactory &getFactory();
};

#endif /* BACKENDS_COREAUDIO_H */
