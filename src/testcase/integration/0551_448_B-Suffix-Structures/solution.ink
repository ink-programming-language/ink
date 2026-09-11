// Translated from solution.cpp.

func power(a: dynamic, n: dynamic) -> dynamic
{
  var res: dynamic = 1;
  while ((n > 0))
  {
    if ((n & 1))
    {
      res = (((((res % 1000000007)) * ((a % 1000000007)))) % 1000000007);
      n -= 1;
    } else
    {
      a = (((((a % 1000000007)) * ((a % 1000000007)))) % 1000000007);
      n /= 2;
    }
  }
  return res;
}

func is_substring(s1: dynamic, s2: dynamic) -> dynamic
{
  var l1: dynamic = s1.size();
  var l2: dynamic = s2.size();
  if ((l2 > l1))
  {
    return false;
  }
  {
    var i: dynamic = 0;
    while ((i < (l1 - l2)))
    {
      var sub: dynamic = s1.substr(i, (i + l2));
      if ((sub == s2))
      {
        return true;
      }
      i += 1;
    }
  }
  return false;
}

func is_subsequence(s1: dynamic, s2: dynamic) -> dynamic
{
  var l1: dynamic = s1.size();
  var l2: dynamic = s2.size();
  if ((l2 > l1))
  {
    return false;
  }
  var i: dynamic = 0;
  var j: dynamic = 0;
  while (((i < l1) && (j < l2)))
  {
    if ((s1[i] == s2[j]))
    {
      j += 1;
    }
    i += 1;
  }
  if ((j == l2))
  {
    return true;
  } else
  {
    return false;
  }
}

func solve() -> dynamic
{
  var s1: dynamic = cpp_uninitialized();
  var s2: dynamic = cpp_uninitialized();
  read(s1, s2);
  var s3: dynamic = s1;
  var s4: dynamic = s2;
  if (is_subsequence(s1, s2))
  {
    write("automaton");
  } else
  {
    sort(s3.begin(), s3.end());
    sort(s4.begin(), s4.end());
    if ((s3 == s4))
    {
      write("array");
    } else
    {
      if ((s1.size() >= s2.size()))
      {
        var cnt: dynamic = 0;
        var mp1: dynamic = cpp_uninitialized();
        var mp2: dynamic = cpp_uninitialized();
        {
          var i: dynamic = 0;
          while ((i < s1.size()))
          {
            mp1[s1[i]] += 1;
            i += 1;
          }
        }
        for (var c: dynamic in s2)
        {
          mp2[c] += 1;
        }
        for (var i: dynamic in mp2)
        {
          if ((mp2[i.first] > mp1[i.first]))
          {
            cnt += 1;
          }
        }
        if ((cnt == 0))
        {
          write("both");
        } else
        {
          write("need tree");
        }
      } else
      {
        write("need tree");
      }
    }
  }
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  var t: dynamic = 1;
  while (cpp_update(t, "--"))
  {
    solve();
  }
}
