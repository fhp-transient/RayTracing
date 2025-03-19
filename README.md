目前已经实现diffuse、简单版本的microfacet和mirror：

## diffuse：spp = 10000: (25min)

![D:\Assignment7\binary.png](https://github.com/fhp-transient/RayTracing/blob/master/result/box_diffuse.png)

## microfacet:spp = 1024: (2min) roughness = 0.25

![D:\Assignment7\binary.png](https://github.com/fhp-transient/RayTracing/blob/master/result/sphere_microfacet.png)

## mirror:spp = 1024: (2min)

![D:\Assignment7\binary.png](https://github.com/fhp-transient/RayTracing/blob/master/result/sphere_mirror.png)

zju 秋季作业结果：

## veach:spp = 1024:

![D:\Assignment7\binary.png](https://github.com/fhp-transient/RayTracing/blob/master/result/veach.png)

## cornell:spp = 1024:

![D:\Assignment7\binary.png](https://github.com/fhp-transient/RayTracing/blob/master/result/cornell.png)

## bathroom:spp = 1024:

![D:\Assignment7\binary.png](https://github.com/fhp-transient/RayTracing/blob/master/result/bathroom.png)

## 优点：

- 手动增加texture，原框架是没有的
- 使用改进的ggx(依然不完美)
- 使用omp和bvh加速

## 不足：

- 纹理没有阴影
- bathroom不知为何有黑的区域
- veach高光不足

## 总结：

方法都是一样的，只是pdf与material的定义不同，个人认为没有必要深究结果到达与目标一致，通过这次作业对蒙特卡洛光线追踪有了深入了解即可。
