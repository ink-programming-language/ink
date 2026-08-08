// Translated from solution.cpp.

var N = (1e5 + 1010);

var L = cpp_array(N);

var R = cpp_array(N);

func main()
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var q: dynamic;
  var ans = 0;
  var s: dynamic;
  var sr: dynamic;
  read(s);
  sr = s;
  reverse(sr.begin(), sr.end());
  read(q);
  while (cpp_update(q, "--"))
  {
    var t: dynamic;
    read(t);
    var a = ((t + cpp_char("#")) + s);
    reverse(t.begin(), t.end());
    var b = ((t + cpp_char("#")) + sr);
    {
      var i = 1;
      while ((i < a.size()))
      {
        var j = L[(i - 1)];
        while ((j && (a[i] != a[j])))
        {
          j = L[(j - 1)];
        }
        if ((a[i] == a[j]))
        {
          j += 1;
        }
        L[i] = j;
        j = R[(i - 1)];
        while ((j && (b[i] != b[j])))
        {
          j = R[(j - 1)];
        }
        if ((b[i] == b[j]))
        {
          j += 1;
        }
        R[i] = j;
        i += 1;
      }
    }
    {
      var i = (t.size() + 1);
      while ((i < a.size()))
      {
        if ((L[i] == t.size()))
        {
          L[i] = 0;
        }
        if ((R[i] == t.size()))
        {
          R[i] = 0;
        }
        L[i] = max(L[i], L[(i - 1)]);
        R[i] = max(R[i], R[(i - 1)]);
        i += 1;
      }
    }
    {
      var i = 1;
      while ((i < s.size()))
      {
        if ((t.size() <= (L[(t.size() + i)] + R[((t.size() + s.size()) - i)])))
        {
          ans += 1;
          break;
        }
        i += 1;
      }
    }
  }
  write(ans);
  return 0;
}
