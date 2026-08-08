// Translated from solution.cpp.

func FastMax(x: dynamic, y: dynamic)
{
  return (((((((y - x)) >> ((32 - 1)))) & ((x ^ y)))) ^ y);
}

func FastMin(x: dynamic, y: dynamic)
{
  return (((((((y - x)) >> ((32 - 1)))) & ((x ^ y)))) ^ x);
}

var N: dynamic;

var Q: dynamic;

var A = cpp_array((1000007 + 7));

var mp: dynamic;

var wh: dynamic;

var qr: dynamic;

var BIT_vol = cpp_array((1000007 + 7));

var BIT_cnt = cpp_array((1000007 + 7));

func Update(B: dynamic, I: dynamic, v: dynamic)
{
  while (cpp_binary(I, "and", (I <= 1000007)))
  {
    B[I] += v;
    I += (I & (-I));
  }
}

func Find(B: dynamic, I: dynamic)
{
  var s = 0;
  while (I)
  {
    s += B[I];
    I -= (I & (-I));
  }
  return s;
}

func isPos(I: dynamic, v: dynamic, ans: dynamic)
{
  var c = Find(BIT_cnt, I);
  var tot = Find(BIT_vol, I);
  var Lim = ((wh[I] * c) - tot);
  if ((Lim > v))
  {
    ans = min(ans, cpp_cast(wh[I]));
    return true;
  } else
  {
    ans = min(ans, (wh[I] + ((((1.0 * v) - Lim)) / c)));
    return false;
  }
}

func main(argument_0: dynamic)
{
  var i: dynamic;
  var j: dynamic;
  var t: dynamic;
  var p: dynamic;
  var x: dynamic;
  var v: dynamic;
  var Icase: dynamic;
  var k = 0;
  scanf("%I64d%I64d", (&N), (&Q));
  mp[-1] = cpp_assign(mp[0], "=", cpp_assign(mp[1000000007], "=", 0));
  {
    i = 1;
    while ((i <= N))
    {
      scanf("%I64d", (&A[i]));
      mp[A[i]] = 0;
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= Q))
    {
      scanf("%I64d", (&t));
      if ((t == 1))
      {
        scanf("%I64d%I64d", (&p), (&x));
        qr.push_back(make_pair(1, make_pair(p, x)));
        mp[x] = 0;
      } else
      {
        scanf("%I64d", (&v));
        qr.push_back(make_pair(2, make_pair(v, 0)));
      }
      i += 1;
    }
  }
  var it = mp.begin();
  i = 0;
  while ((it != mp.end()))
  {
    it->second = i;
    wh.push_back(it->first);
    i += 1;
    it += 1;
  }
  {
    i = 1;
    while ((i <= N))
    {
      Update(BIT_vol, mp[A[i]], A[i]);
      Update(BIT_cnt, mp[A[i]], 1);
      i += 1;
    }
  }
  {
    i = 0;
    while ((i < qr.size()))
    {
      t = qr[i].first;
      if ((t == 1))
      {
        p = qr[i].second.first;
        x = qr[i].second.second;
        Update(BIT_vol, mp[A[p]], (-A[p]));
        Update(BIT_cnt, mp[A[p]], -1);
        A[p] = x;
        Update(BIT_vol, mp[A[p]], A[p]);
        Update(BIT_cnt, mp[A[p]], 1);
      } else
      {
        v = qr[i].second.first;
        var lo = 1;
        var hi = mp.size();
        var ans = 1e17;
        while ((lo <= hi))
        {
          var mid = (((lo + hi)) / 2);
          if (isPos(mid, v, ans))
          {
            hi = (mid - 1);
          } else
          {
            lo = (mid + 1);
          }
        }
        printf("%.7lf\n", ans);
      }
      i += 1;
    }
  }
  return 0;
}
