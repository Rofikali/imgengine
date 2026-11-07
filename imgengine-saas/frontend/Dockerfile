FROM node:24-alpine AS build
WORKDIR /app
RUN corepack enable
COPY package.json pnpm-lock.yaml pnpm-workspace.yaml ./
RUN pnpm install --frozen-lockfile
COPY . .
RUN pnpm run build

FROM node:24-alpine
WORKDIR /app
COPY --from=build /app/.output ./.output
COPY docker-entrypoint.sh /usr/local/bin/imgengine-web-entrypoint
RUN chmod 0555 /usr/local/bin/imgengine-web-entrypoint
ENV HOST=0.0.0.0
ENV PORT=3000
EXPOSE 3000
ENTRYPOINT ["/usr/local/bin/imgengine-web-entrypoint"]
