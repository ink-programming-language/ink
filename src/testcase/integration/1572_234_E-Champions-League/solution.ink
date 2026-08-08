// Translated from solution.cpp.

var X: dynamic;

var A: dynamic;

var B: dynamic;

var C: dynamic;

func next()
{
  var y = ((((X * A) + B)) % C);
  X = y;
  return X;
}

var MAXN = 100;

var names = cpp_array(MAXN, MAXN);

var scores = cpp_array(MAXN);

var ar = cpp_array(MAXN);

func cmp(a: dynamic, b: dynamic)
{
  return (scores[a] > scores[b]);
}

var bask1: dynamic;

var bask2: dynamic;

var bask3: dynamic;

var bask4: dynamic;

func rm(v: dynamic, i: dynamic)
{
  var tmp = v[i];
  v.erase((v.begin() + i));
  return tmp;
}

var N: dynamic;

var s: dynamic;

func main()
{
  if (fopen("input.txt", "r"))
  {
    freopen("input.txt", "r", stdin);
    freopen("output.txt", "w", stdout);
  }
  scanf("%d", (&N));
  s = (N / 4);
  scanf("%d %d %d %d", (&X), (&A), (&B), (&C));
  {
    var i = 0;
    while ((i < N))
    {
      scanf("%s", names[i]);
      scanf("%d", (scores + i));
      ar[i] = i;
      i += 1;
    }
  }
  sort(ar, (ar + N), cmp);
  {
    var i = 0;
    while ((i < s))
    {
      bask1.push_back(ar[i]);
      i += 1;
    }
  }
  {
    var i = s;
    while ((i < (2 * s)))
    {
      bask2.push_back(ar[i]);
      i += 1;
    }
  }
  {
    var i = (2 * s);
    while ((i < (3 * s)))
    {
      bask3.push_back(ar[i]);
      i += 1;
    }
  }
  {
    var i = (3 * s);
    while ((i < (4 * s)))
    {
      bask4.push_back(ar[i]);
      i += 1;
    }
  }
  var groupC = cpp_char("A");
  while (s)
  {
    var r1 = rm(bask1, (next() % s));
    var r2 = rm(bask2, (next() % s));
    var r3 = rm(bask3, (next() % s));
    var r4 = rm(bask4, (next() % s));
    s -= 1;
    printf("Group %c:\n%s\n%s\n%s\n%s\n", cpp_update(groupC, "++"), names[r1], names[r2], names[r3], names[r4]);
  }
  return 0;
}
