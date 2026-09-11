// Translated from solution.cpp.

var maxn: dynamic = (2e5 + 7);

var inf: dynamic = (1e18 + 7);

var mod: dynamic = (1e9 + 7);

var q: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(10);

var b: dynamic = cpp_array(50);

var c: dynamic = cpp_array(10);

var ci: dynamic = cpp_array(10);

func wer(nub: dynamic, k: dynamic) -> dynamic
{
  var ct: dynamic = 0;
  while ((ci[ct] < k))
  {
    ct += 1;
  }
  k = (k - ci[(ct - 1)]);
  var ki: dynamic = ((((k - 1)) / ct) + (c[ct] / 9));
  var cnt: dynamic = (ct - (((k - 1)) % ct));
  while (cpp_update(cnt, "--"))
  {
    ki = (ki / 10);
  }
  return (ki % 10);
}

func main() -> dynamic
{
  var h: dynamic = 9;
  ci[0] = 0;
  c[0] = 0;
  {
    var i: dynamic = 1;
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
    var i: dynamic = 1;
    while ((i <= 9))
    {
      a[i] = (a[(i - 1)] + ((((((2 * ci[(i - 1)]) + (i * c[i])) + i)) * c[i]) / 2));
      i += 1;
    }
  }
  var th: dynamic = 1;
  {
    var i: dynamic = 1;
    while ((i <= 9))
    {
      {
        var j: dynamic = 1;
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
    var ct: dynamic = 0;
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
    var l: dynamic = 1;
    var r: dynamic = c[ct];
    var mid: dynamic = cpp_uninitialized();
    var kh: dynamic = cpp_uninitialized();
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
    var ans: dynamic = (((c[ct] / 9) - 1) + kh);
    k = (k - ((((((2 * ci[(ct - 1)]) + (ct * kh))) * ((kh - 1))) / 2)));
    write(wer(ans, k), "\n");
  }
  return 0;
}
