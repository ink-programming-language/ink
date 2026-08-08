// Translated from solution.cpp.

func sqr(x: dynamic)
{
  return cpp_expression("#includ");
}

class ii
{
  var x: dynamic;
  var y: dynamic;
  var id: dynamic;
}

var num = cpp_array(100005);

var n: dynamic;

var m: dynamic;

var i: dynamic;

var j: dynamic;

var ans: dynamic;

var fa = cpp_array(100005);

class bian
{
  var x: dynamic;
  var y: dynamic;
  var z: dynamic;
}

var bi = cpp_array(200005);

func cmp(x: dynamic, y: dynamic)
{
  return (x.x < y.x);
}

func cmp2(x: dynamic, y: dynamic)
{
  return (x.y < y.y);
}

func cmp3(x: dynamic, y: dynamic)
{
  return (x.id < y.id);
}

func cmp4(x: dynamic, y: dynamic)
{
  return (x.z < y.z);
}

func find(x: dynamic)
{
  if ((x == fa[x]))
  {
    return x;
  }
  return cpp_assign(fa[x], "=", find(fa[x]));
}

func dis(x: dynamic, y: dynamic)
{
  return min(abs((num[x].x - num[y].x)), abs((num[x].y - num[y].y)));
}

func main()
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
