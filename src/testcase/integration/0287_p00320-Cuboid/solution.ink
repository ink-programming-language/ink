// Translated from solution.cpp.

func overload4(cpp_1: dynamic, cpp_2: dynamic, cpp_3: dynamic, cpp_4: dynamic, name: dynamic)
{
  return cpp_expression("#inc");
}

func overload3(cpp_1: dynamic, cpp_2: dynamic, cpp_3: dynamic, name: dynamic)
{
  return cpp_expression("#inc");
}

func rep1(n: dynamic)
{
  return cpp_expression("#include <");
}

func rep2(i: dynamic, n: dynamic)
{
  return cpp_expression("#include <bi");
}

func rep3(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(ll i=a;i<b;++i)");
}

func rep4(i: dynamic, a: dynamic, b: dynamic, c: dynamic)
{
  cpp_macro("for(ll i=a;i<b;i+=c)");
}

func rep()
{
  return cpp_expression("#include <bits/stdc++.h> using namespace std; using namespac");
}

func rrep1(n: dynamic)
{
  return cpp_expression("#include <b");
}

func rrep2(i: dynamic, n: dynamic)
{
  return cpp_expression("#include <bit");
}

func rrep3(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(ll i=b-1;i>=a;i--)");
}

func rrep4(i: dynamic, a: dynamic, b: dynamic, c: dynamic)
{
  cpp_macro("for(ll i=a+(b-a-1)/c*c;i>=a;i-=c)");
}

func rrep()
{
  return cpp_expression("#include <bits/stdc++.h> using namespace std; using namespace st");
}

func each(i: dynamic, a: dynamic)
{
  return cpp_expression("#include <bits/");
}

func sum()
{
  return cpp_expression("#include <bits/stdc++.h> using nam");
}

func range(i: dynamic)
{
  return cpp_expression("#include <bits/stdc++");
}

func range2(i: dynamic, k: dynamic)
{
  return cpp_expression("#include <bits/stdc++.h>");
}

func range3(i: dynamic, a: dynamic, b: dynamic)
{
  return cpp_expression("#include <bits/stdc++.h> us");
}

func range()
{
  return cpp_expression("#include <bits/stdc++.h> using namespace std; using namespa");
}

var LINF = 0x3fffffffffffffff;

var MOD = 1000000007;

var MODD = 0x3b800001;

var INF = 0x3fffffff;

func yes(i: dynamic)
{
  return cpp_expression("#include <bits/st");
}

func Yes(i: dynamic)
{
  return cpp_expression("#include <bits/st");
}

func YES(i: dynamic)
{
  return cpp_expression("#include <bits/st");
}

func Possible(i: dynamic)
{
  return cpp_expression("#include <bits/stdc++.h> using");
}

func unless(a: dynamic)
{
  return cpp_expression("#include");
}

func INT()
{
  cpp_macro("int __VA_ARGS__;in(__VA_ARGS__)");
}

func LL()
{
  cpp_macro("ll __VA_ARGS__;in(__VA_ARGS__)");
}

func STR()
{
  cpp_macro("string __VA_ARGS__;in(__VA_ARGS__)");
}

func CHR()
{
  cpp_macro("char __VA_ARGS__;in(__VA_ARGS__)");
}

func DBL()
{
  cpp_macro("double __VA_ARGS__;in(__VA_ARGS__)");
}

func vec(type_cpp: dynamic, name: dynamic)
{
  cpp_macro("vector<type> name(__VA_ARGS__);");
}

func VEC(type_cpp: dynamic, name: dynamic, size: dynamic)
{
  cpp_macro("vector<type> name(size);in(name)");
}

func vv(type_cpp: dynamic, name: dynamic, h: dynamic)
{
  return cpp_expression("#include <bits/stdc++.h> using namespace std; using n");
}

var pll = cpp_expression("#include <b");

class SETTINGS
{
  func SETTINGS()
  {
      cin.tie(0);
      cout.tie(0);
      ios.sync_with_stdio(0);
      write(fixed, setprecision(20));
    }
}

var SETTINGS: dynamic;

func update_min(mn: dynamic, cnt: dynamic)
{
  if ((mn > cnt))
  {
    mn = cnt;
    return 1;
  } else
  {
    return 0;
  }
}

func update_max(mx: dynamic, cnt: dynamic)
{
  if ((mx < cnt))
  {
    mx = cnt;
    return 1;
  } else
  {
    return 0;
  }
}

func max(vec: dynamic)
{
  return (*max_element(range(vec)));
}

func gcd(a: dynamic, b: dynamic)
{
  if ((a == b))
  {
    return a;
  } else
  {
    return gcd(b, ((((a - 1)) % b) + 1));
  }
}

func in_cpp()
{
}

func operator_shift_right(is: dynamic, vec: dynamic)
{
  for (var x in vec)
  {
    (is >> x);
  }
  return is;
}

func operator_shift_right(is: dynamic, p: dynamic)
{
  (is >> p.first);
  (is >> p.second);
  return is;
}

func operator_shift_left(os: dynamic, vec: dynamic)
{
  (os << vec[0]);
  rep(i, 1, vec.size());
  {
    ((os << cpp_char(" ")) << vec[i]);
  }
  return os;
}

func operator_shift_left(os: dynamic, vec: dynamic)
{
  (os << vec[0]);
  rep(i, 1, vec.size());
  {
    ((os << cpp_char(" ")) << vec[i]);
  }
  return os;
}

func operator_shift_left(os: dynamic, p: dynamic)
{
  (((os << p.first) << " ") << p.second);
  return os;
}

func in_cpp(head: dynamic, tail: dynamic...)
{
  read(head);
  in_cpp(cpp_expand(move(tail)));
}

func out(t: dynamic)
{
  write(t, "\n");
}

func out()
{
  write(cpp_char("\n"));
}

func out(head: dynamic, tail: dynamic...)
{
  write(head, cpp_char(" "));
  out(cpp_expand(move(tail)));
}

func main()
{
  cpp_statement("vv(int,a,6,2); in(a); rep(6)sort(range(a[i])); sort(range(a)); if(a[0]!=a[1]||a[2]!=a[3]||a[4]!=a[5])return puts(\"no\")&0; rep(2)rep(j,2)");
  if ((((a[0][0] == a[2][(1 ^ i)]) && (a[2][(0 ^ i)] == a[4][(1 ^ j)])) && (a[4][(0 ^ j)] == a[0][1])))
  {
    return (puts("yes") & 0);
  }
  yes(0);
}
