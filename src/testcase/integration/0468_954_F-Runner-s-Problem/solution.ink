// Translated from solution.cpp.

var mod = 1000000007;

func add(a: dynamic, b: dynamic)
{
  var res = (a + b);
  return if ((res >= mod)) (res - mod) else res;
}

func mul(a: dynamic, b: dynamic)
{
  return ((a * b) % mod);
}

func matmul(a: dynamic, b: dynamic)
{
  var n = (cpp_cast((a).size()));
  var m = (cpp_cast((b).size()));
  var o = (cpp_cast((b[0]).size()));
  {
    var i = (0);
    while ((i < (n)))
    {
      {
        var j = (0);
        while ((j < (o)))
        {
          {
            var k = (0);
            while ((k < (m)))
            {
              ans[i][j] = add(ans[i][j], mul(a[i][k], b[k][j]));
              k += 1;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  return ans;
}

func powmod(a: dynamic, b: dynamic)
{
  assert((b >= 0));
  var n = (cpp_cast((a).size()));
  {
    var i = (0);
    while ((i < (n)))
    {
      ans[i][i] = 1;
      i += 1;
    }
  }
  {
    while (b)
    {
      if ((b & 1))
      {
        ans = matmul(ans, a);
      }
      a = matmul(a, a);
      b >>= 1;
    }
  }
  return ans;
}

var event: dynamic;

var cnt = cpp_array(4);

var n: dynamic;

var m: dynamic;

func main()
{
  scanf("%d%lld", (&n), (&m));
  {
    var i = (0);
    while ((i < (n)))
    {
      var l: dynamic;
      var r: dynamic;
      var a: dynamic;
      scanf("%d%lld%lld", (&a), (&l), (&r));
      a -= 1;
      l -= 1;
      event.push_back(make_pair(l, make_pair(0, a)));
      event.push_back(make_pair(r, make_pair(1, a)));
      i += 1;
    }
  }
  event.push_back(make_pair(1, make_pair(0, 3)));
  event.push_back(make_pair(m, make_pair(1, 3)));
  sort((event).begin(), (event).end());
  var cur = [[0], [1], [0]];
  {
    var i = 0;
    while ((i < (((cpp_cast((event).size())) - 1))))
    {
      var t = event[i].second.first;
      var of = event[i].second.second;
      if ((t == 0))
      {
        cnt[of] += 1;
      } else
      {
        cnt[of] -= 1;
      }
      var now = cpp_construct(3, vector(3));
      if ((cnt[0] == 0))
      {
        now[0] = [1, 1, 0];
      }
      if ((cnt[1] == 0))
      {
        now[1] = [1, 1, 1];
      }
      if ((cnt[2] == 0))
      {
        now[2] = [0, 1, 1];
      }
      var len = (event[(i + 1)].first - event[i].first);
      cur = matmul(powmod(now, len), cur);
      i += 1;
    }
  }
  printf("%lld", cur[1][0]);
}
