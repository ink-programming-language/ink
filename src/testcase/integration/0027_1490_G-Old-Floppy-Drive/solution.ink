// Translated from solution.cpp.

var IOS: dynamic = cpp_expression("#include<iostream> #include<b");

func REP(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(_loop_int i=0;i<(_loop_int)(n);++i)");
}

func FOR(i: dynamic, a: dynamic, b: dynamic) -> dynamic
{
  cpp_macro("for(_loop_int i=(_loop_int)(a);i<(_loop_int)(b);++i)");
}

func FORR(i: dynamic, a: dynamic, b: dynamic) -> dynamic
{
  cpp_macro("for(_loop_int i=(_loop_int)(b)-1;i>=(_loop_int)(a);--i)");
}

func DEB(x: dynamic) -> dynamic
{
  cpp_macro("cout << #x << \" \" << x << endl;");
}

func DEB_VEC(v: dynamic) -> dynamic
{
  cpp_macro("cout<<#v<<\":\";REP(i,v.size())cout<<\" \"<<v[i];cout<<endl");
}

func ALL(a: dynamic) -> dynamic
{
  return cpp_expression("#include<iostream> #i");
}

func CHMIN(a: dynamic, b: dynamic) -> dynamic
{
  return cpp_expression("#include<iostr");
}

func CHMAX(a: dynamic, b: dynamic) -> dynamic
{
  return cpp_expression("#include<iostr");
}

func solve() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(n, m);
  var a: dynamic = cpp_array(n);
  REP(i, n);
  read(a[i]);
  var peak: dynamic = LONG_MIN;
  var sum: dynamic = 0;
  var mp: dynamic = cpp_uninitialized();
  var offset: dynamic = sum;
  var res: dynamic = cpp_uninitialized();
  write("\n");
}

func main() -> dynamic
{
  var int_cpp: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    solve();
  }
  return 0;
}

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    sum += a[i];
    if (((mp.count(sum) == 0) && (peak < sum)))
    {
      mp[sum] = (i + 1);
    }
    CHMAX(peak, sum);
  }

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    var x: dynamic = cpp_uninitialized();
    read(x);
    if (res.count(x))
    {
      write(res[x], " ");
      continue;
    }
    if (((peak < x) && (offset <= 0)))
    {
      write(-1, " ");
      continue;
    }
    var k: dynamic =  ((peak >= x)) ? 0 : (((((x - peak) + offset) - 1)) / offset);
    var sec: dynamic =  ((peak >= x)) ? -1 : ((n * k) - 1);
    var koff: dynamic = (k * offset);
    var search: dynamic = (x - koff);
    var it: dynamic = mp.lower_bound(search);
    sec += it->second;
    write(sec, " ");
    res[x] = sec;
  }
