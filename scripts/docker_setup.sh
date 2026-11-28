# 把容器 stop → commit → 带新 restart policy run
docker commit roto-builder roto-builder-fixed
docker rm roto-builder
docker run -d --name roto-builder \
    --restart=unless-stopped \
    --hostname rk3576-builder \
    --network host \
    -v /data/workspace/roto_mobile_sales_cart_ws:/ws \
    -v /etc/localtime:/etc/localtime:ro \
    roto-builder-fixed sleep infinity