// Translated from solution.cpp.

var cpp_name = cpp_expression("#include<bits/stdc++.h> #defi");

func REP(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i=0;i<(int)(n);i++)");
}

func main()
{
  cpp_name;
  var e: dynamic;
  while (cpp_comma((cin >> e), (e != 0)))
  {
    var m = (1 << 29);
    REP(i, 101);
    {
      if ((((i * i) * i) > e))
      {
        break;
      }
      REP(j, 1001);
      {
        if (((((i * i) * i) + (j * j)) > e))
        {
          break;
        }
        m = min(m, ((i + j) + (((e - ((i * i) * i)) - (j * j)))));
      }
    }
    write(m, "\n");
  }
}
