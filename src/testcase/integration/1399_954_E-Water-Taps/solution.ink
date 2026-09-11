// Translated from solution.cpp.

var N: dynamic = (2e5 + 7);

var n: dynamic = cpp_uninitialized();

var T: dynamic = cpp_uninitialized();

var t: dynamic = cpp_array(N);

var x: dynamic = cpp_array(N);

var sa: dynamic = cpp_uninitialized();

var sb: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_uninitialized();

var a: dynamic = cpp_uninitialized();

var b: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  read(n, T);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(x[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(t[i]);
      if ((t[i] == T))
      {
        ans += x[i];
      } else
      {
        if ((t[i] < T))
        {
          a.push_back([(T - t[i]), x[i]]);
          sa += (((T - t[i])) * x[i]);
        }
        if ((t[i] > T))
        {
          b.push_back([(t[i] - T), x[i]]);
          sb += (((t[i] - T)) * x[i]);
        }
      }
      i += 1;
    }
  }
  sort(a.begin(), a.end());
  sort(b.begin(), b.end());
  var can: dynamic = min(sa, sb);
  {
    var i: dynamic = 0;
    while ((i < a.size()))
    {
      if ((can < (a[i].first * a[i].second)))
      {
        ans += ((1.0 * can) / a[i].first);
        break;
      }
      can -= (a[i].first * a[i].second);
      ans += a[i].second;
      i += 1;
    }
  }
  can = min(sa, sb);
  {
    var i: dynamic = 0;
    while ((i < b.size()))
    {
      if ((can < (b[i].first * b[i].second)))
      {
        ans += ((1.0 * can) / b[i].first);
        break;
      }
      can -= (b[i].first * b[i].second);
      ans += b[i].second;
      i += 1;
    }
  }
  write(fixed, setprecision(10), ans);
  return 0;
}
