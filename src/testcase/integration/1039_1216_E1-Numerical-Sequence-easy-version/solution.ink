// Translated from solution.cpp.

var maxn = (2e5 + 7);

var inf = (1e18 + 7);

var mod = (1e9 + 7);

var q: dynamic;

var k: dynamic;

var a = cpp_array(10);

var b = cpp_array(50);

var c = cpp_array(10);

var ci = cpp_array(10);

func wer(nub: dynamic, k: dynamic)
{
  var ct = 0;
  while ((ci[ct] < k))
  {
    ct += 1;
  }
  k = (k - ci[(ct - 1)]);
  var ki = ((((k - 1)) / ct) + (c[ct] / 9));
  var cnt = (ct - (((k - 1)) % ct));
  while (cpp_update(cnt, "--"))
  {
    ki = (ki / 10);
  }
  return (ki % 10);
}

func main()
{
  var h = 9;
  ci[0] = 0;
  c[0] = 0;
  {
    var i = 1;
    while ((i <= 9))
    {
      c[i] = h;
      ci[i] = (ci[(i - 1)] + (i * c[i]));
      h *= 10;
      i += 1;
    }
  }
  a[0] = 0;
  {
    var i = 1;
    while ((i <= 9))
    {
      a[i] = (a[(i - 1)] + ((((((2 * ci[(i - 1)]) + (i * c[i])) + i)) * c[i]) / 2));
      i += 1;
    }
  }
  var th = 1;
  {
    var i = 1;
    while ((i <= 9))
    {
      {
        var j = 1;
        while ((j <= i))
        {
          b[cpp_update(th, "++")] = j;
          j += 1;
        }
      }
      i += 1;
    }
  }
  read(q);
  while (cpp_update(q, "--"))
  {
    read(k);
    var ct = 0;
    while ((a[ct] < k))
    {
      ct += 1;
    }
    if ((ct == 1))
    {
      write(b[k], "\n");
      continue;
    }
    k = (k - a[(ct - 1)]);
    var l = 1;
    var r = c[ct];
    var mid: dynamic;
    var kh: dynamic;
    while ((l <= r))
    {
      mid = (((l + r)) / 2);
      if ((((((((2 * ci[(ct - 1)]) + (ct * mid)) + ct)) * mid) / 2) >= k))
      {
        r = (mid - 1);
        kh = mid;
      } else
      {
        l = (mid + 1);
      }
    }
    var ans = (((c[ct] / 9) - 1) + kh);
    k = (k - ((((((2 * ci[(ct - 1)]) + (ct * kh))) * ((kh - 1))) / 2)));
    write(wer(ans, k), "\n");
  }
  return 0;
}
