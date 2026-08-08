// Translated from solution.cpp.

var IOS = cpp_expression("#include<iostream> #include<b");

func REP(i: dynamic, n: dynamic)
{
  cpp_macro("for(_loop_int i=0;i<(_loop_int)(n);++i)");
}

func FOR(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(_loop_int i=(_loop_int)(a);i<(_loop_int)(b);++i)");
}

func FORR(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(_loop_int i=(_loop_int)(b)-1;i>=(_loop_int)(a);--i)");
}

func DEB(x: dynamic)
{
  cpp_macro("cout << #x << \" \" << x << endl;");
}

func DEB_VEC(v: dynamic)
{
  cpp_macro("cout<<#v<<\":\";REP(i,v.size())cout<<\" \"<<v[i];cout<<endl");
}

func ALL(a: dynamic)
{
  return cpp_expression("#include<iostream> #i");
}

func CHMIN(a: dynamic, b: dynamic)
{
  return cpp_expression("#include<iostr");
}

func CHMAX(a: dynamic, b: dynamic)
{
  return cpp_expression("#include<iostr");
}

func solve()
{
  var n: dynamic;
  var m: dynamic;
  read(n, m);
  var a = cpp_array(n);
  REP(i, n);
  read(a[i]);
  var peak = LONG_MIN;
  var sum = 0;
  var mp: dynamic;
  var offset = sum;
  var res: dynamic;
  write("\n");
}

func main()
{
  var int_cpp: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    solve();
  }
  return 0;
}

func REP(argument_0: dynamic, argument_1: dynamic)
{
    sum += a[i];
    if (((mp.count(sum) == 0) && (peak < sum)))
    {
      mp[sum] = (i + 1);
    }
    CHMAX(peak, sum);
  }

func REP(argument_0: dynamic, argument_1: dynamic)
{
    var x: dynamic;
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
    var k = if ((peak >= x)) 0 else (((((x - peak) + offset) - 1)) / offset);
    var sec = if ((peak >= x)) -1 else ((n * k) - 1);
    var koff = (k * offset);
    var search = (x - koff);
    var it = mp.lower_bound(search);
    sec += it->second;
    write(sec, " ");
    res[x] = sec;
  }
