// Translated from solution.cpp.

func modu(a: dynamic, b: dynamic) -> dynamic
{
  var ans: dynamic = 1;
  while ((b > 0))
  {
    if ((b & 1))
    {
      ans = (((ans * a)) % 1000000007);
    }
    b /= 2;
    a = (((a * a)) % 1000000007);
  }
  return ans;
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var n: dynamic = cpp_uninitialized();
  read(n);
  var arr: dynamic = cpp_array((n + 1));
  var s: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      read(arr[i]);
      s.insert(arr[i]);
      i += 1;
    }
  }
  var cn: dynamic = 0;
  var ans: dynamic = cpp_uninitialized();
  while ((s.find(cn) != s.end()))
  {
    cn += 1;
  }
  ans.push_back(cn);
  cn += 1;
  var i: dynamic = 2;
  while ((i <= n))
  {
    if ((arr[i] == arr[(i - 1)]))
    {
      while ((s.find(cn) != s.end()))
      {
        cn += 1;
      }
      ans.push_back(cn);
      cn += 1;
      i += 1;
    } else
    {
      ans.push_back(arr[(i - 1)]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < ans.size()))
    {
      write(ans[i], " ");
      i += 1;
    }
  }
  write("\n");
  return 0;
}
