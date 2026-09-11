// Translated from solution.cpp.

func REP(i: dynamic, n: dynamic, N: dynamic) -> dynamic
{
  cpp_macro("for(int i=n;i<N;i++)");
}

func p(S: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/");
}

func ck(a: dynamic, b: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/");
}

func abcgene(anss: dynamic) -> dynamic
{
  var q: dynamic = cpp_uninitialized();
  q.push(anss);
  var c: dynamic = ["A", "B", "C"];
  var m: dynamic = cpp_uninitialized();
  m[anss] = true;
  while ((!q.empty()))
  {
    var s: dynamic = q.front();
    q.pop();
    if ((s == "ABC"))
    {
      return "Yes";
    }
    REP(j, 0, 3);
    {
      var tmps: dynamic = s;
      var cnt: dynamic = 0;
      REP(i, 0, (cpp_cast(tmps.size()) - 2));
      {
        if ((((tmps[i] == cpp_char("A")) && (tmps[(i + 1)] == cpp_char("B"))) && (tmps[(i + 2)] == cpp_char("C"))))
        {
          tmps.replace(i, 3, c[j]);
          cnt += 1;
        }
      }
      if ((cnt != count(tmps.begin(), tmps.end(), (cpp_char("A") + j))))
      {
        continue;
      }
      if ((!m[tmps]))
      {
        m[tmps] = true;
        q.push(tmps);
      }
    }
  }
  return "No";
}

func main() -> dynamic
{
  var ans: dynamic = cpp_uninitialized();
  read(ans);
  p(abcgene(ans));
  return 0;
}
