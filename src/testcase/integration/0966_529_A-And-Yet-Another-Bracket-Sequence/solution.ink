// Translated from solution.cpp.

var maxn: dynamic = 1000047;

var s: dynamic = cpp_array(maxn);

var n: dynamic = cpp_uninitialized();

func utried() -> dynamic
{
  var por: dynamic = cpp_uninitialized();
  var otv: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      if ((s[i] == cpp_char("(")))
      {
        por.push_back(i);
        lepsich[i] = 0;
        otv += 1;
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      if ((s[i] == cpp_char(")")))
      {
        por.push_back(i);
        lepsich[i] = otv;
      }
      i += 1;
    }
  }
  {
    var sl: dynamic = 1;
    while ((sl < n))
    {
      var off: dynamic = cpp_construct(n, 0);
      {
        var i: dynamic = 0;
        while ((i < n))
        {
          var ind: dynamic = ((((por[i] - sl) + n)) % n);
          npor[(lepsich[ind] + off[lepsich[ind]])] = ind;
          off[lepsich[ind]] += 1;
          i += 1;
        }
      }
      por = npor;
      nlepsich[por[0]] = 0;
      {
        var i: dynamic = 1;
        while ((i < n))
        {
          if (((lepsich[por[i]] == lepsich[por[(i - 1)]]) && (lepsich[(((por[i] + sl)) % n)] == lepsich[(((por[(i - 1)] + sl)) % n)])))
          {
            nlepsich[por[i]] = nlepsich[por[(i - 1)]];
          } else
          {
            nlepsich[por[i]] = i;
          }
          i += 1;
        }
      }
      lepsich = nlepsich;
      sl *= 2;
    }
  }
  return por;
}

func main() -> dynamic
{
  scanf("%s", s);
  {
    n = 0;
    while ((s[n] != 0))
    {
      n += 1;
    }
  }
  var depth: dynamic = cpp_construct((n + 1), 0);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if ((s[(i - 1)] == cpp_char("(")))
      {
        depth[i] = (depth[(i - 1)] + 1);
      } else
      {
        depth[i] = (depth[(i - 1)] - 1);
      }
      i += 1;
    }
  }
  var minpred: dynamic = cpp_construct((n + 1), 0);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      minpred[i] = min(depth[i], minpred[(i - 1)]);
      i += 1;
    }
  }
  var minpo: dynamic = cpp_construct((n + 1), 1023456789);
  {
    var i: dynamic = (n - 1);
    while ((i >= 0))
    {
      minpo[i] = min(depth[(i + 1)], minpo[(i + 1)]);
      i -= 1;
    }
  }
  var can_be: dynamic = cpp_construct(n, 0);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      if ((depth[n] < 0))
      {
        can_be[i] = ((((minpo[i] - depth[i]) >= depth[n]) && (minpred[i] >= depth[i])));
      } else
      {
        can_be[i] = (((minpo[i] >= depth[i]) && ((minpred[i] + depth[n]) >= depth[i])));
      }
      i += 1;
    }
  }
  var por: dynamic = utried();
  var st: dynamic = -1;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      if (can_be[por[i]])
      {
        st = por[i];
        break;
      }
      i += 1;
    }
  }
  if ((depth[n] < 0))
  {
    {
      var i: dynamic = 0;
      while ((i < ((0 - depth[n]))))
      {
        printf("(");
        i += 1;
      }
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      printf("%c", s[(((i + st)) % n)]);
      i += 1;
    }
  }
  if ((depth[n] > 0))
  {
    {
      var i: dynamic = 0;
      while ((i < depth[n]))
      {
        printf(")");
        i += 1;
      }
    }
  }
  printf("\n");
  return 0;
}
