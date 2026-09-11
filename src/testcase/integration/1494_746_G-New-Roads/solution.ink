// Translated from solution.cpp.

var v: dynamic = cpp_array(200005);

var ar: dynamic = cpp_array(200005);

func print(t: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i <= t))
    {
      {
        var j: dynamic = 0;
        while ((j < ar[i]))
        {
          write(v[i][j], " ");
          j += 1;
        }
      }
      write("\n");
      i += 1;
    }
  }
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var mn: dynamic = cpp_uninitialized();
  var mx: dynamic = cpp_uninitialized();
  var f: dynamic = 1;
  scanf("%d%d%d", (&n), (&t), (&k));
  mx = (n - t);
  ar[0] = 1;
  {
    var i: dynamic = 1;
    while ((i <= t))
    {
      scanf("%d", (&ar[i]));
      i += 1;
    }
  }
  mn = ar[t];
  {
    var i: dynamic = 0;
    while ((i < t))
    {
      v[i].push_back(ar[(i + 1)]);
      {
        var j: dynamic = 1;
        while ((j < ar[i]))
        {
          v[i].push_back(0);
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var j: dynamic = 1;
    while ((j <= ar[t]))
    {
      v[t].push_back(0);
      j += 1;
    }
  }
  if (((k >= mn) && (k <= mx)))
  {
    var z: dynamic = ((n - ar[t]) - t);
    k = (k - ar[t]);
    z = (z - k);
    {
      var i: dynamic = (t - 1);
      while (((i >= 1) && (z != 0)))
      {
        var vl: dynamic = ar[(i + 1)];
        {
          var j: dynamic = 0;
          while (((j < ar[i]) && (z != 0)))
          {
            if ((vl != 0))
            {
              if ((v[i][j] == 0))
              {
                z -= 1;
              }
              v[i][j] = 1;
              vl -= 1;
            }
            j += 1;
          }
        }
        v[i][0] += vl;
        i -= 1;
      }
    }
    if ((z != 0))
    {
      f = 0;
    }
  } else
  {
    f = 0;
  }
  if ((f == 0))
  {
    write(-1, "\n");
  } else
  {
    write(n, "\n");
    var a: dynamic = 1;
    {
      var i: dynamic = 0;
      while ((i < t))
      {
        mn = (a + ar[i]);
        {
          var j: dynamic = 0;
          while ((j < ar[i]))
          {
            {
              var k: dynamic = 0;
              while ((k < v[i][j]))
              {
                write(a, " ", mn, "\n");
                k += 1;
                mn += 1;
              }
            }
            a += 1;
            j += 1;
          }
        }
        i += 1;
      }
    }
  }
}
