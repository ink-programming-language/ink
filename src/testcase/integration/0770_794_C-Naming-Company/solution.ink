// Translated from solution.cpp.

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  var first: dynamic;
  var second: dynamic;
  read(first, second);
  var X: dynamic;
  var Y: dynamic;
  for (var ch in first)
  {
    X.insert(ch);
  }
  for (var ch in second)
  {
    Y.insert(ch);
  }
  var n = (cpp_cast((first).size()));
  var half = ((n / 2) + ((n & 1)));
  while (((cpp_cast((X).size())) != half))
  {
    X.erase((cpp_update((X.end()), "--")));
  }
  while (((cpp_cast((Y).size())) != (n - half)))
  {
    Y.erase(Y.begin());
  }
  var left = 0;
  var right = (n - 1);
  var turn = 1;
  {
    var i = 0;
    while ((i < n))
    {
      if (turn)
      {
        if (Y.empty())
        {
          ans[left] = (*X.begin());
          break;
        }
        var mx = (*(cpp_update((Y.end()), "--")));
        var mn = (*X.begin());
        if ((mx > mn))
        {
          ans[cpp_update(left, "++")] = mn;
          X.erase(X.begin());
        } else
        {
          mx = (*(cpp_update((X.end()), "--")));
          ans[cpp_update(right, "--")] = mx;
          X.erase(X.lower_bound(mx));
        }
      } else
      {
        var mx = (*(cpp_update((Y.end()), "--")));
        if (X.empty())
        {
          ans[left] = mx;
          break;
        }
        var mn = (*X.begin());
        if ((mx > mn))
        {
          ans[cpp_update(left, "++")] = mx;
          Y.erase(Y.lower_bound(mx));
        } else
        {
          mn = (*Y.begin());
          ans[cpp_update(right, "--")] = mn;
          Y.erase(Y.begin());
        }
      }
      turn ^= 1;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      write(ans[i]);
      i += 1;
    }
  }
  write("\n");
}
