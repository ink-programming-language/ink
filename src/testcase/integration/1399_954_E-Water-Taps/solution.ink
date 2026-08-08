// Translated from solution.cpp.

var N = (2e5 + 7);

var n: dynamic;

var T: dynamic;

var t = cpp_array(N);

var x = cpp_array(N);

var sa: dynamic;

var sb: dynamic;

var ans: dynamic;

var a: dynamic;

var b: dynamic;

func main()
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  read(n, T);
  {
    var i = 0;
    while ((i < n))
    {
      read(x[i]);
      i += 1;
    }
  }
  {
    var i = 0;
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
  var can = min(sa, sb);
  {
    var i = 0;
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
    var i = 0;
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
