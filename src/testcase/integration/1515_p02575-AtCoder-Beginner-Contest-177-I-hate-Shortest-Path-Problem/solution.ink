// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for (int i = 0; i < (int)(n); i++)");
}

var INF = cpp_expression("#include <");

var MAX = cpp_expression("#includ");

func main()
{
  var h: dynamic;
  var w: dynamic;
  read(h, w);
  var sp: dynamic;
  var diff: dynamic;
  rep(i, w)[i] = i;
  diff.insert(0);
  var ans: dynamic;
  for (var a in ans)
  {
    write(a, "\n");
  }
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic)
{
    var a: dynamic;
    var b: dynamic;
    read(a, b);
    a -= 1;
    b -= 1;
    var m = -1;
    var itr = sp.lower_bound(a);
    while (((itr != sp.end()) && (itr->first <= (b + 1))))
    {
      m = max(m, itr->second);
      diff.erase(diff.find((itr->first - itr->second)));
      itr = sp.erase(itr);
    }
    if (((m != -1) && (b < (w - 1))))
    {
      sp[(b + 1)] = m;
      diff.insert(((b + 1) - sp[(b + 1)]));
    }
    ans.push_back(if (diff.empty()) -1 else (((*diff.begin()) + i) + 1));
  }
