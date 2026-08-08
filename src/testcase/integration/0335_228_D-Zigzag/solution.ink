// Translated from solution.cpp.

func swap(x: dynamic, y: dynamic)
{
  var t = x;
  x = y;
  y = t;
}

func max(x: dynamic, y: dynamic)
{
  return if ((x > y)) x else y;
}

func min(x: dynamic, y: dynamic)
{
  return if ((x < y)) x else y;
}

var inf = 0x3F3F3F3F;

var M = (100000 + 5);

var T: dynamic;

var cas: dynamic;

var n: dynamic;

var m: dynamic;

var a: dynamic;

var sum = cpp_array(11, 5, (M << 2));

var s = cpp_array(M, 5);

var cf = [2, 4, 6, 8, 10];

func preSof()
{
  {
    var z = 2;
    while ((z <= 6))
    {
      var md = (((z - 1)) << 1);
      {
        var i = 1;
        while ((i < 13))
        {
          var j = (i % md);
          if ((!j))
          {
            s[(z - 2)][(i - 1)] = 2;
          } else if ((j <= z))
          {
            s[(z - 2)][(i - 1)] = j;
          } else
          {
            s[(z - 2)][(i - 1)] = (((z << 1)) - j);
          }
          i += 1;
        }
      }
      z += 1;
    }
  }
  return;
}

func pushUp(llen: dynamic, rt: dynamic)
{
  {
    var z = 2;
    while ((z <= 6))
    {
      {
        var i = 0;
        while ((i < cf[(z - 2)]))
        {
          sum[rt][(z - 2)][i] = (sum[(rt << 1)][(z - 2)][i] + sum[((rt << 1) | 1)][(z - 2)][(((i + llen)) % cf[(z - 2)])]);
          i += 1;
        }
      }
      z += 1;
    }
  }
}

func build(l: dynamic, r: dynamic, rt: dynamic)
{
  if ((l == r))
  {
    scanf("%I64d", (&a));
    {
      var z = 2;
      while ((z <= 6))
      {
        {
          var i = 0;
          while ((i < cf[(z - 2)]))
          {
            sum[rt][(z - 2)][i] = (a * s[(z - 2)][i]);
            i += 1;
          }
        }
        z += 1;
      }
    }
    return;
  }
  var mid = ((l + r) >> 1);
  build(l, mid, (rt << 1));
  build((mid + 1), r, ((rt << 1) | 1));
  pushUp(((mid - l) + 1), rt);
}

func update(l: dynamic, r: dynamic, rt: dynamic, p: dynamic, c: dynamic)
{
  if ((l == r))
  {
    {
      var z = 2;
      while ((z <= 6))
      {
        {
          var i = 0;
          while ((i < cf[(z - 2)]))
          {
            sum[rt][(z - 2)][i] = (c * s[(z - 2)][i]);
            i += 1;
          }
        }
        z += 1;
      }
    }
    return;
  }
  var mid = ((l + r) >> 1);
  if ((p <= mid))
  {
    update(l, mid, (rt << 1), p, c);
  } else
  {
    update((mid + 1), r, ((rt << 1) | 1), p, c);
  }
  pushUp(((mid - l) + 1), rt);
}

func query(l: dynamic, r: dynamic, rt: dynamic, L: dynamic, R: dynamic, z: dynamic, i: dynamic)
{
  if (((L == l) && (r == R)))
  {
    return sum[rt][z][i];
  }
  var mid = ((l + r) >> 1);
  if ((R <= mid))
  {
    return query(l, mid, (rt << 1), L, R, z, i);
  }
  if ((mid < L))
  {
    return query((mid + 1), r, ((rt << 1) | 1), L, R, z, i);
  }
  return (query(l, mid, (rt << 1), L, mid, z, i) + query((mid + 1), r, ((rt << 1) | 1), (mid + 1), R, z, (((((i + mid) - L) + 1)) % cf[z])));
}

func run()
{
  var i: dynamic;
  var j: dynamic;
  var t: dynamic;
  var p: dynamic;
  var v: dynamic;
  var l: dynamic;
  var r: dynamic;
  var z: dynamic;
  build(1, n, 1);
  scanf("%d", (&m));
  while (cpp_update(m, "--"))
  {
    scanf("%d", (&t));
    if ((t == 1))
    {
      scanf("%d%d", (&p), (&v));
      update(1, n, 1, p, cpp_cast(v));
    } else
    {
      scanf("%d%d%d", (&l), (&r), (&z));
      printf("%I64d\n", query(1, n, 1, l, r, (z - 2), 0));
    }
  }
}

func main()
{
  preSof();
  while ((~scanf("%d", (&n))))
  {
    run();
  }
  return 0;
}
