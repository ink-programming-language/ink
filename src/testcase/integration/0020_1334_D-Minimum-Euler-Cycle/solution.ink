// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var tc: dynamic = 0;

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  if ((tc < 0))
  {
    write("TC!\n");
    cin.ignore(1e8);
  } else if ((!tc))
  {
    read(tc);
  }
  while (cpp_update(tc, "--"))
  {
    solve();
  }
  return 0;
}

func solve() -> dynamic
{
  var l: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  read(n, l, r);
  l -= 1;
  r -= 1;
  var a: dynamic = 1;
  while (1)
  {
    if ((l > (2 * ((n - a)))))
    {
      l -= (2 * ((n - a)));
      r -= (2 * ((n - a)));
      a += 1;
    } else
    {
      break;
    }
  }
  var ans: dynamic = cpp_uninitialized();
  var b: dynamic = (a + 1);
  while ((ans.size() <= (r + 1)))
  {
    ans.push_back(a);
    ans.push_back(b);
    b += 1;
    if ((b == (n + 1)))
    {
      a += 1;
      if ((a == n))
      {
        ans.push_back(1);
        break;
      }
      b = (a + 1);
    }
  }
  {
    var i: dynamic = l;
    while ((i <= r))
    {
      write(ans[i], " ");
      i += 1;
    }
  }
  write("\n");
  return 0;
}
