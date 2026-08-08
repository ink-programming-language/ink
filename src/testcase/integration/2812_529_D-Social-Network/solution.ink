// Translated from solution.cpp.

var MN = 100111;

var n: dynamic;

var M: dynamic;

var T: dynamic;

var a = cpp_array(MN);

var bit = cpp_array(MN);

var res = cpp_array(MN);

func get(u: dynamic)
{
  var res = 0;
  while ((u > 0))
  {
    res += bit[u];
    u -= (((u) & ((-(u)))));
  }
  return res;
}

func update(u: dynamic, val: dynamic)
{
  while ((u < MN))
  {
    bit[u] += val;
    u += (((u) & ((-(u)))));
  }
}

func main()
{
  while ((scanf("%d%d%d", (&n), (&M), (&T)) == 3))
  {
    {
      var i = (1);
      var b = (n);
      while ((i <= b))
      {
        var h: dynamic;
        var m: dynamic;
        var s: dynamic;
        scanf("%d:%d:%d", (&h), (&m), (&s));
        a[i] = ((((h * 3600) + (m * 60)) + s) + 1);
        i += 1;
      }
    }
    memset(bit, 0, cpp_sizeof(bit));
    var ok = false;
    {
      var i = (1);
      var b = (n);
      while ((i <= b))
      {
        var x = ((a[i] - T) + 1);
        if ((x < 0))
        {
          x = 0;
        }
        var has = get(a[i]);
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
        var i = (1);
        var b = (n);
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
