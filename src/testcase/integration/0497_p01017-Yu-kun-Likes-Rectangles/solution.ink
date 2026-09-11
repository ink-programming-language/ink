// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=0;i<n;++i)");
}

func REP1(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=1;i<=n;++i)");
}

func ALL(c: dynamic) -> dynamic
{
  return cpp_expression("#include <iostream> #");
}

var a: dynamic = cpp_array(50, 50);

var b: dynamic = cpp_array(50, 50);

var c: dynamic = cpp_array(50, 50);

func main() -> dynamic
{
  var ha: dynamic = cpp_uninitialized();
  var wa: dynamic = cpp_uninitialized();
  var hc: dynamic = cpp_uninitialized();
  var wc: dynamic = cpp_uninitialized();
  read(ha, wa);
  rep(i, ha);
  rep(j, wa);
  read(a[i][j]);
  rep(i, ha);
  rep(j, wa);
  read(b[i][j]);
  read(hc, wc);
  rep(i, hc);
  rep(j, wc);
  read(c[i][j]);
  var ans: dynamic = -1000000000;
  rep(i, ((ha - hc) + 1));
  rep(j, ((wa - wc) + 1));
  {
    var tmp: dynamic = 0;
    var flag: dynamic = true;
    if (flag)
    {
      ans = max(ans, tmp);
    }
  }
  if ((ans == -1000000000))
  {
    write("NA", "\n");
  } else
  {
    write(ans, "\n");
  }
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
        if ((c[k][l] != b[(i + k)][(j + l)]))
        {
          flag = false;
          break;
        }
        tmp += a[(i + k)][(j + l)];
      }

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      if ((!flag))
      {
        break;
      }
    }
