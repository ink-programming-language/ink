// Translated from solution.cpp.

var MAXN: dynamic = 105;

var rng: dynamic = cpp_construct(chrono.steady_clock.now().time_since_epoch().count());

func last_of(a: dynamic, b: dynamic) -> dynamic
{
  assert((b != 1));
  if (((a % ((b - 1))) == 0))
  {
    return ((a / ((b - 1))) - 1);
  }
  return (a / ((b - 1)));
}

func cei(a: dynamic, b: dynamic) -> dynamic
{
  return ((((a + b) - 1)) / b);
}

var n: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var h: dynamic = cpp_array(MAXN);

var val: dynamic = cpp_array(MAXN);

func solve() -> dynamic
{
  read(n, k);
  var lb: dynamic = cpp_uninitialized();
  var ev: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      read(h[i]);
      var d: dynamic = 1;
      while (1)
      {
        lb.push_back(d);
        ev.push_back(pair(d, i));
        if ((cei(h[i], d) == 1))
        {
          break;
        }
        d = (last_of(h[i], cei(h[i], d)) + 1);
      }
      i += 1;
    }
  }
  lb.push_back(1e13);
  sort((lb).begin(), (lb).end());
  lb.resize((unique((lb).begin(), (lb).end()) - lb.begin()));
  sort((ev).begin(), (ev).end());
  var cpp_ptr: dynamic = 0;
  var sum: dynamic = 0;
  var sumh: dynamic = 0;
  var ans: dynamic = 1;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      sumh += h[i];
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < (cpp_cast(lb.size()) - 1)))
    {
      var d: dynamic = lb[i];
      var r: dynamic = (lb[(i + 1)] - 1);
      while (((cpp_ptr < ev.size()) && (ev[cpp_ptr].first <= d)))
      {
        var id: dynamic = ev[cpp_ptr].second;
        sum -= val[id];
        val[id] = cei(h[id], d);
        sum += val[id];
        cpp_ptr += 1;
      }
      var best: dynamic = (((k + sumh)) / sum);
      if ((best >= d))
      {
        ans = max(ans, min(best, r));
      }
      i += 1;
    }
  }
  write(ans, cpp_char("\n"));
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  var tc: dynamic = 1;
  {
    var i: dynamic = 1;
    while ((i <= tc))
    {
      solve();
      i += 1;
    }
  }
  return 0;
}
