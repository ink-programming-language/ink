// Translated from solution.cpp.

var MN: dynamic = 100111;

var n: dynamic = cpp_uninitialized();

var M: dynamic = cpp_uninitialized();

var T: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(MN);

var bit: dynamic = cpp_array(MN);

var res: dynamic = cpp_array(MN);

func get(u: dynamic) -> dynamic
{
  var res: dynamic = 0;
  while ((u > 0))
  {
    res += bit[u];
    u -= (((u) & ((-(u)))));
  }
  return res;
}

func update(u: dynamic, val: dynamic) -> dynamic
{
  while ((u < MN))
  {
    bit[u] += val;
    u += (((u) & ((-(u)))));
  }
}

func main() -> dynamic
{
  while ((scanf("%d%d%d", (&n), (&M), (&T)) == 3))
  {
    {
      var i: dynamic = (1);
      var b: dynamic = (n);
      while ((i <= b))
      {
        var h: dynamic = cpp_uninitialized();
        var m: dynamic = cpp_uninitialized();
        var s: dynamic = cpp_uninitialized();
        scanf("%d:%d:%d", (&h), (&m), (&s));
        a[i] = ((((h * 3600) + (m * 60)) + s) + 1);
        i += 1;
      }
    }
    memset(bit, 0, cpp_sizeof(bit));
    var ok: dynamic = false;
    {
      var i: dynamic = (1);
      var b: dynamic = (n);
      while ((i <= b))
      {
        var x: dynamic = ((a[i] - T) + 1);
        if ((x < 0))
        {
          x = 0;
        }
        var has: dynamic = get(a[i]);
        if (x)
        {
          has -= get((x - 1));
        }
        if (((has + 1) >= M))
        {
          ok = true;
        }
        if ((has < M))
        {
          res[i] = (res[(i - 1)] + 1);
          update(a[i], 1);
        } else
        {
          res[i] = res[(i - 1)];
          update(a[(i - 1)], -1);
          update(a[i], 1);
        }
        i += 1;
      }
    }
    if ((!ok))
    {
      write("No solution", "\n");
    } else
    {
      printf("%d\n", res[n]);
      {
        var i: dynamic = (1);
        var b: dynamic = (n);
        while ((i <= b))
        {
          printf("%d\n", res[i]);
          i += 1;
        }
      }
    }
  }
  return 0;
}
