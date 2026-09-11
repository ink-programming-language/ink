// Translated from solution.cpp.

var cin: dynamic = cpp_expression("#in");

var cout: dynamic = cpp_expression("#inc");

var fin: dynamic = cpp_construct("test.in");

var fout: dynamic = cpp_construct("test.out");

var t: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(200001);

var f: dynamic = cpp_array(200001);

var nra: dynamic = cpp_uninitialized();

var p: dynamic = cpp_array(100001);

var k: dynamic = cpp_uninitialized();

func rasp(mij: dynamic) -> dynamic
{
  var r: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= k))
    {
      if (((p[i] - mij) < 0))
      {
        r += (p[i] * f[p[i]]);
      } else
      {
        r += (((p[i] - mij)) * f[p[i]]);
      }
      i += 1;
    }
  }
  return r;
}

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(null);
  read(t);
  {
    var tt: dynamic = 1;
    while ((tt <= t))
    {
      read(n);
      var r: dynamic = (n + 1);
      k = 0;
      {
        var i: dynamic = 1;
        while ((i <= n))
        {
          read(a[i]);
          f[i] = 0;
          i += 1;
        }
      }
      sort((a + 1), ((a + n) + 1));
      nra = 0;
      a[0] = a[1];
      a[(n + 1)] = (a[n] + 1);
      {
        var i: dynamic = 1;
        while ((i <= (n + 1)))
        {
          if ((a[i] == a[(i - 1)]))
          {
            nra += 1;
          } else
          {
            f[nra] += 1;
            nra = 1;
          }
          i += 1;
        }
      }
      {
        var i: dynamic = 1;
        while ((i <= n))
        {
          if (f[i])
          {
            p[cpp_update(k, "++")] = i;
          }
          i += 1;
        }
      }
      {
        var i: dynamic = 1;
        while ((i <= k))
        {
          r = min(r, rasp(p[i]));
          i += 1;
        }
      }
      write(r, cpp_char("\n"));
      tt += 1;
    }
  }
  return 0;
}
