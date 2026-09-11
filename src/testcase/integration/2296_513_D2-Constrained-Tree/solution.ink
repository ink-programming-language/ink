// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var c: dynamic = cpp_uninitialized();

var nod: dynamic = cpp_uninitialized();

var gnext: dynamic = cpp_array(2, (1 << 20));

var ans: dynamic = cpp_uninitialized();

func PANIC() -> dynamic
{
  write("IMPOSSIBLE\n");
  exit(0);
}

func dfs(r: dynamic) -> dynamic
{
  var onod: dynamic = cpp_update(nod, "++");
  if ((onod >= n))
  {
    PANIC();
  }
  if (gnext[onod][0].empty())
  {
    ans.push_back(onod);
  } else
  {
    if (((!gnext[onod][1].empty()) && ((*gnext[onod][0].rbegin()) >= (*gnext[onod][1].begin()))))
    {
      PANIC();
    }
    if (((*gnext[onod][0].begin()) < nod))
    {
      PANIC();
    }
    dfs((*gnext[onod][0].rbegin()));
    ans.push_back(onod);
  }
  if ((!gnext[onod][1].empty()))
  {
    if (((*gnext[onod][1].begin()) < nod))
    {
      PANIC();
    }
    dfs(max(r, (*gnext[onod][1].rbegin())));
  } else if ((nod <= r))
  {
    dfs(r);
  }
}

func main() -> dynamic
{
  ios.sync_with_stdio(0);
  read(n, c);
  {
    var i: dynamic = 0;
    while ((i < c))
    {
      var str: dynamic = cpp_uninitialized();
      var a: dynamic = cpp_uninitialized();
      var b: dynamic = cpp_uninitialized();
      read(a, b, str);
      a -= 1;
      b -= 1;
      if ((a >= b))
      {
        PANIC();
      }
      if ((str[0] == cpp_char("L")))
      {
        gnext[a][0].insert(b);
      } else
      {
        gnext[a][1].insert(b);
      }
      i += 1;
    }
  }
  dfs((n - 1));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      write((ans[i] + 1), cpp_char(" "));
      i += 1;
    }
  }
  write(cpp_char("\n"));
  return 0;
}
