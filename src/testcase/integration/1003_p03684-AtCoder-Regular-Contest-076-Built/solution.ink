// Translated from solution.cpp.

func sqr(x: dynamic) -> dynamic
{
  return cpp_expression("#includ");
}

class ii
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var id: dynamic = cpp_uninitialized();
}

var num: dynamic = cpp_array(100005);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var i: dynamic = cpp_uninitialized();

var j: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_uninitialized();

var fa: dynamic = cpp_array(100005);

class bian
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var z: dynamic = cpp_uninitialized();
}

var bi: dynamic = cpp_array(200005);

func cmp(x: dynamic, y: dynamic) -> dynamic
{
  return (x.x < y.x);
}

func cmp2(x: dynamic, y: dynamic) -> dynamic
{
  return (x.y < y.y);
}

func cmp3(x: dynamic, y: dynamic) -> dynamic
{
  return (x.id < y.id);
}

func cmp4(x: dynamic, y: dynamic) -> dynamic
{
  return (x.z < y.z);
}

func find(x: dynamic) -> dynamic
{
  if ((x == fa[x]))
  {
    return x;
  }
  return cpp_assign(fa[x], "=", find(fa[x]));
}

func dis(x: dynamic, y: dynamic) -> dynamic
{
  return min(abs((num[x].x - num[y].x)), abs((num[x].y - num[y].y)));
}

func main() -> dynamic
{
  read(n);
  {
    i = 1;
    while ((i <= n))
    {
      read(num[i].x, num[i].y);
      num[i].id = i;
      i += 1;
    }
  }
  sort((num + 1), ((num + n) + 1), cmp);
  {
    i = 1;
    while ((i < n))
    {
      bi[i] = [num[i].id, num[(i + 1)].id, 0];
      i += 1;
    }
  }
  sort((num + 1), ((num + n) + 1), cmp2);
  {
    i = 1;
    while ((i < n))
    {
      bi[((i + n) - 1)] = [num[i].id, num[(i + 1)].id, 0];
      i += 1;
    }
  }
  sort((num + 1), ((num + n) + 1), cmp3);
  {
    i = 1;
    while ((i <= ((2 * n) - 2)))
    {
      bi[i].z = dis(bi[i].x, bi[i].y);
      i += 1;
    }
  }
  sort((bi + 1), (((bi + n) + n) - 1), cmp4);
  {
    i = 1;
    while ((i <= n))
    {
      fa[i] = i;
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= ((n + n) - 2)))
    {
      if ((find(bi[i].x) != find(bi[i].y)))
      {
        fa[find(bi[i].x)] = find(bi[i].y);
        ans += bi[i].z;
      }
      i += 1;
    }
  }
  write(ans);
  return 0;
}
