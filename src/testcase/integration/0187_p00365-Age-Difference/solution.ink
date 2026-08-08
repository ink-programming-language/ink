// Translated from solution.cpp.

var fst = cpp_expression("#incl");

var snd = cpp_expression("#inclu");

var pb = cpp_expression("#include<");

func FOR(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(auto i=(a);i<(b);++i)");
}

func RFOR(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(auto i=(a);i>=(b);--i)");
}

func REP(i: dynamic, a: dynamic)
{
  cpp_macro("for(long i=0;i<(a);++i)");
}

func RREP(i: dynamic, a: dynamic)
{
  cpp_macro("for(long i=(a);i>=0;--i)");
}

func EACH(i: dynamic, a: dynamic)
{
  cpp_macro("for(auto (i)=(a).begin(),_END=(a).end();i!=_END;++i)");
}

func REACH(i: dynamic, a: dynamic)
{
  cpp_macro("for(auto (i)=(a).rbegin(),_END=(a).rend();i!=_END;++i)");
}

func ALL(a: dynamic)
{
  return cpp_expression("#include<bits/stdc++");
}

func RALL(a: dynamic)
{
  return cpp_expression("#include<bits/stdc++.h");
}

func EXIST(a: dynamic, x: dynamic)
{
  return cpp_expression("#include<bits/stdc++.h>");
}

func SORT(a: dynamic)
{
  return cpp_expression("#include<bits/stdc++.h> // Shrot");
}

func UNIQUE(a: dynamic)
{
  cpp_macro("std::sort((a).begin(), a.end()), a.erase(std::unique((a).begin(), a.end()), a.end());");
}

func SUM(a: dynamic)
{
  cpp_macro("std::accumulate((a).begin(), (a).end(), 0);");
}

var OPT = cpp_expression("#include<bits/st");

var debug = true;

func MSG(s: dynamic)
{
  cpp_macro("if(debug){std::cout << s << std::endl;}");
}

func DEBUG(x: dynamic)
{
  cpp_macro("if(debug){std::cout << \"debug(\" << #x << \"): \" << x << std::endl;}");
}

func main()
{
  var p1: dynamic;
  var p2: dynamic;
  read(p1.fst, p1.snd.fst, p1.snd.snd, p2.fst, p2.snd.fst, p2.snd.snd);
  if ((p1 < p2))
  {
    swap(p1, p2);
  }
  var ans = ((p1.fst - p2.fst) + cpp_cast(((p1.snd > p2.snd))));
  write(ans, "\n");
}
