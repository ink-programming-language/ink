// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for (int i = 0; i < (int)(n); i++)");
}

var INF: dynamic = cpp_expression("#include <");

var MAX: dynamic = cpp_expression("#includ");

func main() -> dynamic
{
  var h: dynamic = cpp_uninitialized();
  var w: dynamic = cpp_uninitialized();
  read(h, w);
  var sp: dynamic = cpp_uninitialized();
  var diff: dynamic = cpp_uninitialized();
  rep(i, w)[i] = i;
  diff.insert(0);
  var ans: dynamic = cpp_uninitialized();
  for (var a: dynamic in ans)
  {
    write(a, "\n");
  }
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    var a: dynamic = cpp_uninitialized();
    var b: dynamic = cpp_uninitialized();
    read(a, b);
    a -= 1;
    b -= 1;
    var m: dynamic = -1;
    var itr: dynamic = sp.lower_bound(a);
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
    ans.push_back( (diff.empty()) ? -1 : (((*diff.begin()) + i) + 1));
  }
