// Translated from solution.cpp.

var MAXN = 105;

var rng = cpp_construct(chrono.steady_clock.now().time_since_epoch().count());

func last_of(a: dynamic, b: dynamic)
{
  assert((b != 1));
  if (((a % ((b - 1))) == 0))
  {
    return ((a / ((b - 1))) - 1);
  }
  return (a / ((b - 1)));
}

func cei(a: dynamic, b: dynamic)
{
  return ((((a + b) - 1)) / b);
}

var n: dynamic;

var k: dynamic;

var h = cpp_array(MAXN);

var val = cpp_array(MAXN);

func solve()
{
  read(n, k);
  var lb: dynamic;
  var ev: dynamic;
  {
    var i = 1;
    while ((i <= n))
    {
      read(h[i]);
      var d = 1;
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
  var ptr = 0;
  var sum = 0;
  var sumh = 0;
  var ans = 1;
  {
    var i = 1;
    while ((i <= n))
    {
      sumh += h[i];
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < (cpp_cast(lb.size()) - 1)))
    {
      var d = lb[i];
      var r = (lb[(i + 1)] - 1);
      while (((ptr < ev.size()) && (ev[ptr].first <= d)))
      {
        var id = ev[ptr].second;
        sum -= val[id];
        val[id] = cei(h[id], d);
        sum += val[id];
        ptr += 1;
      }
      var best = (((k + sumh)) / sum);
      if ((best >= d))
      {
        ans = max(ans, min(best, r));
      }
      i += 1;
    }
  }
  write(ans, cpp_char("\n"));
}

func main()
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  var tc = 1;
  {
    var i = 1;
    while ((i <= tc))
    {
      solve();
      i += 1;
    }
  }
  return 0;
}
