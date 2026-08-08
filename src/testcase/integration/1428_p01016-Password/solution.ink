// Translated from solution.cpp.

var lint = dynamic;

var P = cpp_expression("#include<bits/");

var LLP = cpp_expression("#include<bits/stdc++.h> us");

func REP(i: dynamic, x: dynamic, n: dynamic)
{
  cpp_macro("for(int i = (x), i##_len = (int)(n) ; i < i##_len ; ++i)");
}

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i = 0, i##_len = (int)(n) ; i < i##_len ; ++i)");
}

func repr(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i = (int)(n) - 1 ; i >= 0 ; --i)");
}

func SORT(x: dynamic)
{
  return cpp_expression("#include<bits/stdc++.h> usin");
}

func SORT_INV(x: dynamic)
{
  return cpp_expression("#include<bits/stdc++.h> using");
}

var IINF = (1e9 + 100);

var LLINF = (2e18 + 129);

var MOD = (1e9 + 7);

var dx4 = [1, 0, -1, 0];

var dy4 = [0, 1, 0, -1];

var dx8 = [1, 1, 0, -1, -1, -1, 0, 1];

var dy8 = [0, -1, -1, -1, 0, 1, 1, 1];

var EPS = 1e-8;

func main()
{
  cin.tie(0);
  ios.sync_with_stdio(false);
  var a: dynamic;
  var b: dynamic;
  read(a, b);
  var n = ((cpp_cast(a.size()) - cpp_cast(b.size())) + 1);
  var ans = false;
  if (ans)
  {
    write("Yes", "\n");
  } else
  {
    write("No", "\n");
  }
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic)
{
    var flag = true;
    rep(j, b.size());
    {
      flag &= (((a[(i + j)] == b[j]) || (b[j] == cpp_char("_"))));
    }
    ans |= flag;
  }
