// Translated from solution.cpp.

var N: dynamic = (1e5 + 1010);

var L: dynamic = cpp_array(N);

var R: dynamic = cpp_array(N);

func main() -> dynamic
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var q: dynamic = cpp_uninitialized();
  var ans: dynamic = 0;
  var s: dynamic = cpp_uninitialized();
  var sr: dynamic = cpp_uninitialized();
  read(s);
  sr = s;
  reverse(sr.begin(), sr.end());
  read(q);
  while (cpp_update(q, "--"))
  {
    var t: dynamic = cpp_uninitialized();
    read(t);
    var a: dynamic = ((t + cpp_char("#")) + s);
    reverse(t.begin(), t.end());
    var b: dynamic = ((t + cpp_char("#")) + sr);
    {
      var i: dynamic = 1;
      while ((i < a.size()))
      {
        var j: dynamic = L[(i - 1)];
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
      var i: dynamic = (t.size() + 1);
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
      var i: dynamic = 1;
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
