FROM    --platform=linux/arm64 alpine:3.16.2 as base

WORKDIR /base
COPY    . ./
RUN     apk add --no-cache --virtual build-tooling \
          bash \
          build-base \
          wget \
    &&  make \
    &&  bash models/download-ggml-model.sh base.en \
    &&  apk del build-tooling

FROM    --platform=linux/arm64 alpine:3.16.2 as deliverable

WORKDIR /vidyard/whisper-project
RUN     apk add --no-cache ffmpeg
COPY    --from=base /base/main ./
COPY    --from=base /base/models/ggml-base.en.bin ./models/
