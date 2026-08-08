// Translated from solution.cpp.

var nmax = 2009;

var a = cpp_array(2009, 2009);

var ui = cpp_array(nmax);

var uj = cpp_array(nmax);

var n: dynamic;

func inc(a: dynamic)
{
  a += 1;
  a = (a & 1);
}

func main()
{
  var ans = 0;
  scanf("%d\n", (&n));
  var s = cpp_array(nmax);
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%s", s);
      {
        var j = 1;
        while ((j <= n))
        {
          a[i][j] = (s[(j - 1)] - cpp_char("0"));
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var k = n;
    while ((k >= 1))
    {
      var p = 0;
      {
        var i = 1;
        while ((i < k))
        {
          if ((((p & 1)) == 1))
          {
            inc(a[i][k]);
          }
          if ((((ui[i] & 1)) == 1))
          {
            inc(a[i][k]);
          }
          if ((a[i][k] != 0))
          {
            inc(p);
            inc(ui[i]);
            inc(a[i][k]);
            ans += 1;
          }
          i += 1;
        }
      }
      if ((((p & 1)) == 1))
      {
        inc(a[k][k]);
      }
      if ((((ui[k] & 1)) == 1))
      {
        inc(a[k][k]);
      }
      p = 0;
      {
        var j = 1;
        while ((j < k))
        {
          if ((((p & 1)) == 1))
          {
            inc(a[k][j]);
          }
          if ((((uj[j] & 1)) == 1))
          {
            inc(a[k][j]);
          }
          if ((a[k][j] != 0))
          {
            inc(p);
            inc(uj[j]);
            inc(a[k][j]);
            ans += 1;
          }
          j += 1;
        }
      }
      if ((((p & 1)) == 1))
      {
        inc(a[k][k]);
      }
      if ((((uj[k] & 1)) == 1))
      {
        inc(a[k][k]);
      }
      if ((a[k][k] != 0))
      {
        ans += 1;
        inc(a[k][k]);
      }
      k -= 1;
    }
  }
  write(ans, "\n");
  return 0;
}
