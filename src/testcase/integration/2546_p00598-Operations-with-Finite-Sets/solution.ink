// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i=0;i<(n);i++)");
}

var idx: dynamic;

var s = cpp_array(2000);

var S = cpp_array(5);

var U: dynamic;

func calc_c(A: dynamic)
{
  var B: dynamic;
  rep(i, U.size());
  if ((!binary_search(A.begin(), A.end(), U[i])))
  {
    B.push_back(U[i]);
  }
  return B;
}

func calc_u(A: dynamic, B: dynamic)
{
  var C: dynamic;
  rep(i, A.size()).push_back(A[i]);
  rep(i, B.size()).push_back(B[i]);
  sort(C.begin(), C.end());
  C.erase(unique(C.begin(), C.end()), C.end());
  return C;
}

func calc_i(A: dynamic, B: dynamic)
{
  return calc_c(calc_u(calc_c(A), calc_c(B)));
}

func calc_d(A: dynamic, B: dynamic)
{
  return calc_i(A, calc_c(B));
}

func calc_s(A: dynamic, B: dynamic)
{
  return calc_u(calc_d(A, B), calc_d(B, A));
}

func expr()
{
  var A = set1();
  while ((s[idx] && (s[idx] != cpp_char(")"))))
  {
    var op = s[cpp_update(idx, "++")];
    var B = set1();
    if ((op == cpp_char("u")))
    {
      A = calc_u(A, B);
    } else if ((op == cpp_char("i")))
    {
      A = calc_i(A, B);
    } else if ((op == cpp_char("d")))
    {
      A = calc_d(A, B);
    } else
    {
      A = calc_s(A, B);
    }
  }
  return A;
}

func set1()
{
  if ((s[idx] == cpp_char("c")))
  {
    idx += 1;
    return calc_c(set1());
  } else
  {
    return set2();
  }
}

func set2()
{
  var A: dynamic;
  if ((s[idx] == cpp_char("(")))
  {
    idx += 1;
    A = expr();
    idx += 1;
  } else
  {
    A = alpha();
  }
  return A;
}

func alpha()
{
  return S[(s[cpp_update(idx, "++")] - cpp_char("A"))];
}

func main()
{
  while (1)
  {
    U.clear();
    while (1)
    {
      var c: dynamic;
      var n: dynamic;
      if ((!(~scanf(" %c%d", (&c), (&n)))))
      {
        return 0;
      }
      if ((c == cpp_char("R")))
      {
        break;
      }
      S[(c - cpp_char("A"))].clear();
      sort(S[(c - cpp_char("A"))].begin(), S[(c - cpp_char("A"))].end());
    }
    sort(U.begin(), U.end());
    U.erase(unique(U.begin(), U.end()), U.end());
    scanf("%s", s);
    idx = 0;
    var res = expr();
    if ((res.size() == 0))
    {
      puts("NULL");
    } else
    {
      rep(i, res.size());
    }
    printf("%d%c", res[i], if ((i < (cpp_cast(res.size()) - 1))) cpp_char(" ") else cpp_char("\n"));
  }
}

func rep(argument_0: dynamic, argument_1: dynamic)
{
        var e: dynamic;
        scanf("%d", (&e));
        S[(c - cpp_char("A"))].push_back(e);
        U.push_back(e);
      }
