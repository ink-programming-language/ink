// Translated from solution.cpp.

func getnum()
{
  var res = 0;
  var c: dynamic;
  var sign = 1;
  while ((((cpp_assign(c, "=", getchar())) == cpp_char(" ")) || (c == cpp_char("\n"))))
  {
    c = getchar();
    if (((c == cpp_char(" ")) || (c == cpp_char("\n"))))
    {
      continue;
    } else
    {
      break;
    }
  }
  if ((c == cpp_char("+")))
  {
  } else if ((c == cpp_char("-")))
  {
    sign = -1;
  } else
  {
    res = (c - cpp_char("0"));
  }
  while (1)
  {
    c = getchar();
    if (((c >= cpp_char("0")) && (c <= cpp_char("9"))))
    {
      res = (((res * 10) + c) - cpp_char("0"));
    } else
    {
      break;
    }
  }
  return (res * sign);
}

func getstr(str: dynamic)
{
  var now = true;
  while (now)
  {
    var k = getchar();
    var __cpp_switch_1 = k;
    if (__cpp_switch_1 == cpp_char("\n"))
    {
    }
    else if (__cpp_switch_1 == cpp_char("\t"))
    {
    }
    else if (__cpp_switch_1 == cpp_char("\u{b}"))
    {
    }
    else if (__cpp_switch_1 == cpp_char("\u{0}"))
    {
    }
    else if (__cpp_switch_1 == cpp_char(" "))
    {
    }
    else if (__cpp_switch_1 == EOF)
    {
      now = false;
      break;
    }
    else
    {
      str.push_back(k);
      break;
    }
  }
}

func main()
{
  var k: dynamic;
  var bef = cpp_char("0");
  var right = -1;
  var streak = 0;
  var ans = 1;
  var now = 0;
  while ((((cpp_assign(k, "=", getchar())) != EOF) && (k != cpp_char("\n"))))
  {
    if (((((bef + k) - cpp_char("0")) - cpp_char("0")) == 9))
    {
      streak += 1;
      right = now;
    } else
    {
      if (streak)
      {
        streak = (if ((streak % 2)) 1 else ((streak / 2) + 1));
        ans *= streak;
      }
      streak = 0;
    }
    now += 1;
    bef = k;
  }
  if (streak)
  {
    streak = (if ((streak % 2)) 1 else ((streak / 2) + 1));
    ans *= streak;
  }
  write(ans);
  return 0;
}
