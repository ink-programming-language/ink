// Translated from solution.cpp.

var v: dynamic = cpp_array(100010);

var a: dynamic = cpp_array(100010);

func dfs(x: dynamic) -> dynamic
{
  var ans: dynamic = 1;
  for (var i: dynamic in v[x])
  {
    a[i] = dfs(i);
    ans += a[i];
  }
  if ((ans != 1))
  {
    ans -= 1;
  }
  return ans;
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  scanf("%d", (&n));
  {
    var i: dynamic = (2);
    while ((i <= (n)))
    {
      var x: dynamic = cpp_uninitialized();
      scanf("%d", (&x));
      v[x].push_back(i);
      i += 1;
    }
  }
  a[1] = dfs(1);
  sort((a + 1), ((a + 1) + n));
  {
    var i: dynamic = (1);
    while ((i <= (n)))
    {
      printf("%d%c", a[i], cpp_char(" "));
      i += 1;
    }
  }
}
