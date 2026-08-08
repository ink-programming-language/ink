// Translated from solution.cpp.

var M = (1e6 + 10);

var M2 = (1e3 + 10);

var mod = (1e9 + 7);

var inf = (1e18 + 10);

var a = cpp_array(M);

var suf = cpp_array(M);

var pre = cpp_array(M);

var n: dynamic;

var t = 0;

var ans = make_pair(make_pair(cpp_cast((-inf)), (-inf)), make_pair((-inf), (-inf)));

func check(x: dynamic, ind: dynamic)
{
  var len = ((n - x) + 1);
  var tmp = ((cpp_cast(pre[ind]) - cpp_cast(pre[((ind - len) - 1)])));
  tmp += cpp_cast(suf[x]);
  tmp /= (((len * 2) + 1));
  tmp -= (cpp_cast(a[ind]));
  return tmp;
}

func check2(x: dynamic, ind: dynamic)
{
  var len = ((n - x) + 1);
  var tmp = ((cpp_cast(pre[ind]) - cpp_cast(pre[max(((ind - len) - 2), t)])));
  tmp += cpp_cast(suf[x]);
  tmp /= cpp_cast((((len * 2) + 2)));
  tmp -= (((((cpp_cast(a[ind]) + cpp_cast(a[(ind - 1)]))) / cpp_cast(2))));
  return tmp;
}

func main()
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  read(n);
  {
    var i = 1;
    while ((i <= n))
    {
      read(a[i]);
      a[i] *= 2;
      i += 1;
    }
  }
  sort((a + 1), ((a + n) + 1));
  {
    var i = 1;
    while ((i <= n))
    {
      pre[i] = (pre[(i - 1)] + a[i]);
      i += 1;
    }
  }
  {
    var i = n;
    while ((i >= 1))
    {
      suf[i] = (suf[(i + 1)] + a[i]);
      i -= 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      var lo = (i + 1);
      var hi = (n + 1);
      lo = max(lo, ((n - i) + 2));
      while ((hi > (lo + 2)))
      {
        var m1 = (((lo + (2 * hi))) / 3);
        var m2 = ((((2 * lo) + hi)) / 3);
        if ((check(m1, i) > check(m2, i)))
        {
          lo = (m2 + 1);
        } else
        {
          hi = (m1 - 1);
        }
      }
      var good = 0;
      var all = cpp_cast((-inf));
      {
        var j = lo;
        while ((j <= hi))
        {
          if ((check(j, i) > all))
          {
            all = check(j, i);
            good = j;
          }
          j += 1;
        }
      }
      if ((all > ans.first.first))
      {
        ans.first.first = all;
        ans.first.second = i;
        ans.second.first = 0;
        ans.second.second = good;
      }
      if ((i == 1))
      {
        i += 1;
        continue;
      }
      lo = (i + 1);
      hi = (n + 1);
      lo = max(lo, ((n - i) + 3));
      while ((hi > (lo + 2)))
      {
        var m1 = (((lo + (2 * hi))) / 3);
        var m2 = ((((2 * lo) + hi)) / 3);
        if ((check2(m1, i) > check2(m2, i)))
        {
          lo = (m2 + 1);
        } else
        {
          hi = (m1 - 1);
        }
      }
      good = 0;
      all = cpp_cast((-inf));
      {
        var j = lo;
        while ((j <= hi))
        {
          if ((check2(j, i) >= all))
          {
            all = check2(j, i);
            good = j;
          }
          j += 1;
        }
      }
      if ((all >= ans.first.first))
      {
        ans.first.first = all;
        ans.first.second = i;
        ans.second.first = (i - 1);
        ans.second.second = good;
      }
      i += 1;
    }
  }
  var len = ((n - ans.second.second) + 1);
  if (ans.second.first)
  {
    write(((len * 2) + 2), "\n");
  } else
  {
    write(((len * 2) + 1), "\n");
  }
  if ((!ans.second.first))
  {
    ans.second.first = ans.first.second;
  }
  {
    var i = (ans.second.first - len);
    while ((i <= ans.second.first))
    {
      write((a[i] / 2), " ");
      i += 1;
    }
  }
  if ((ans.second.first != ans.first.second))
  {
    write((a[ans.first.second] / 2), " ");
  }
  {
    var i = ((n - len) + 1);
    while ((i <= n))
    {
      write((a[i] / 2), " ");
      i += 1;
    }
  }
}
