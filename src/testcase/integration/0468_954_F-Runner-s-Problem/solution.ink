// Translated from solution.cpp.

var mod: dynamic = 1000000007;

func add(a: dynamic, b: dynamic) -> dynamic
{
  var res: dynamic = (a + b);
  return  ((res >= mod)) ? (res - mod) : res;
}

func mul(a: dynamic, b: dynamic) -> dynamic
{
  return ((a * b) % mod);
}

func matmul(a: dynamic, b: dynamic) -> dynamic
{
  var n: dynamic = (cpp_cast((a).size()));
  var m: dynamic = (cpp_cast((b).size()));
  var o: dynamic = (cpp_cast((b[0]).size()));
  {
    var i: dynamic = (0);
    while ((i < (n)))
    {
      {
        var j: dynamic = (0);
        while ((j < (o)))
        {
          {
            var k: dynamic = (0);
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

func powmod(a: dynamic, b: dynamic) -> dynamic
{
  assert((b >= 0));
  var n: dynamic = (cpp_cast((a).size()));
  {
    var i: dynamic = (0);
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

var event: dynamic = cpp_uninitialized();

var cnt: dynamic = cpp_array(4);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  scanf("%d%lld", (&n), (&m));
  {
    var i: dynamic = (0);
    while ((i < (n)))
    {
      var l: dynamic = cpp_uninitialized();
      var r: dynamic = cpp_uninitialized();
      var a: dynamic = cpp_uninitialized();
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
  var cur: dynamic = [[0], [1], [0]];
  {
    var i: dynamic = 0;
    while ((i < (((cpp_cast((event).size())) - 1))))
    {
      var t: dynamic = event[i].second.first;
      var of: dynamic = event[i].second.second;
      if ((t == 0))
      {
        cnt[of] += 1;
      } else
      {
        cnt[of] -= 1;
      }
      var now: dynamic = cpp_construct(3, vector(3));
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
      var len: dynamic = (event[(i + 1)].first - event[i].first);
      cur = matmul(powmod(now, len), cur);
      i += 1;
    }
  }
  printf("%lld", cur[1][0]);
}
