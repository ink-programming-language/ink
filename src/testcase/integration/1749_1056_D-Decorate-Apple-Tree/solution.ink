// Translated from solution.cpp.

var v = cpp_array(100010);

var a = cpp_array(100010);

func dfs(x: dynamic)
{
  var ans = 1;
  for (var i in v[x])
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

func main()
{
  var n: dynamic;
  scanf("%d", (&n));
  {
    var i = (2);
    while ((i <= (n)))
    {
      var x: dynamic;
      scanf("%d", (&x));
      v[x].push_back(i);
      i += 1;
    }
  }
  a[1] = dfs(1);
  sort((a + 1), ((a + 1) + n));
  {
    var i = (1);
    while ((i <= (n)))
    {
      printf("%d%c", a[i], cpp_char(" "));
      i += 1;
    }
  }
}
