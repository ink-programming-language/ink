// Translated from solution.cpp.

var cin = cpp_expression("#in");

var cout = cpp_expression("#inc");

var fin = cpp_construct("test.in");

var fout = cpp_construct("test.out");

var t: dynamic;

var n: dynamic;

var a = cpp_array(200001);

var f = cpp_array(200001);

var nra: dynamic;

var p = cpp_array(100001);

var k: dynamic;

func rasp(mij: dynamic)
{
  var r = 0;
  {
    var i = 1;
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

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(null);
  read(t);
  {
    var tt = 1;
    while ((tt <= t))
    {
      read(n);
      var r = (n + 1);
      k = 0;
      {
        var i = 1;
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
        var i = 1;
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
        var i = 1;
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
        var i = 1;
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
