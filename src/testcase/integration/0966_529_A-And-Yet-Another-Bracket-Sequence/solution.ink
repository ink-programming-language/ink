// Translated from solution.cpp.

var maxn = 1000047;

var s = cpp_array(maxn);

var n: dynamic;

func utried()
{
  var por: dynamic;
  var otv = 0;
  {
    var i = 0;
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
    var i = 0;
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
    var sl = 1;
    while ((sl < n))
    {
      var off = cpp_construct(n, 0);
      {
        var i = 0;
        while ((i < n))
        {
          var ind = ((((por[i] - sl) + n)) % n);
          npor[(lepsich[ind] + off[lepsich[ind]])] = ind;
          off[lepsich[ind]] += 1;
          i += 1;
        }
      }
      por = npor;
      nlepsich[por[0]] = 0;
      {
        var i = 1;
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

func main()
{
  scanf("%s", s);
  {
    n = 0;
    while ((s[n] != 0))
    {
      n += 1;
    }
  }
  var depth = cpp_construct((n + 1), 0);
  {
    var i = 1;
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
  var minpred = cpp_construct((n + 1), 0);
  {
    var i = 1;
    while ((i <= n))
    {
      minpred[i] = min(depth[i], minpred[(i - 1)]);
      i += 1;
    }
  }
  var minpo = cpp_construct((n + 1), 1023456789);
  {
    var i = (n - 1);
    while ((i >= 0))
    {
      minpo[i] = min(depth[(i + 1)], minpo[(i + 1)]);
      i -= 1;
    }
  }
  var can_be = cpp_construct(n, 0);
  {
    var i = 0;
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
  var por = utried();
  var st = -1;
  {
    var i = 0;
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
      var i = 0;
      while ((i < ((0 - depth[n]))))
      {
        printf("(");
        i += 1;
      }
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      printf("%c", s[(((i + st)) % n)]);
      i += 1;
    }
  }
  if ((depth[n] > 0))
  {
    {
      var i = 0;
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
