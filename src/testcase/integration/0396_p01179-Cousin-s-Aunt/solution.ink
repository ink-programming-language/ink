// Translated from solution.cpp.

enum Relation
{
  FATHER,
  MOTHER,
  SON,
  DAUGHTER,
  HUSBAND,
  WIFE,
  BROTHER,
  SISTER,
  GRANDFATHER,
  GRANDMOTHER,
  GRANDSON,
  GRANDDAUGHTER,
  UNCLE,
  AUNT,
  NEPHEW,
  NIECE
}

enum Sex
{
  MALE,
  FEMALE
}

class Node
{
  var parent: dynamic;
  var sons: dynamic;
  var daughters: dynamic;
  var distance: dynamic;
  func Node(distance: dynamic)
  {
      this->distance = cpp_construct(distance);
      parent[MALE] = cpp_assign(parent[FEMALE], "=", null);
    }
}

func size(x: dynamic)
{
  return x.size();
}

func split(str: dynamic, delimiter: dynamic)
{
  var result: dynamic;
  {
    var i = 0;
    while ((i < size(str)))
    {
      var word: dynamic;
      {
        while (((i < size(str)) && (str[i] != delimiter)))
        {
          word += str[i];
          i += 1;
        }
      }
      result.push_back(move(word));
      i += 1;
    }
  }
  return result;
}

func startsWith(str: dynamic, prefix: dynamic)
{
  if ((str.size() < prefix.size()))
  {
    return false;
  }
  {
    var i = 0;
    while ((i < size(prefix)))
    {
      if ((str[i] != prefix[i]))
      {
        return false;
      }
      i += 1;
    }
  }
  return true;
}

var relations: dynamic;

var ansMax: dynamic;

var ansMin: dynamic;

func withParent(node: dynamic, sex: dynamic, proc: dynamic)
{
  if (node->parent[sex])
  {
    proc(node->parent[sex]);
  } else
  {
    var n = cpp_construct((node->distance + 1));
    if ((sex == MALE))
    {
      n.sons.push_back(node);
    } else
    {
      n.daughters.push_back(node);
    }
    node->parent[sex] = (&n);
    proc((&n));
    node->parent[sex] = null;
  }
}

func withSon(node: dynamic, sex: dynamic, proc: dynamic)
{
  {
    var i = 0;
    while ((i < size(node->sons)))
    {
      proc(node->sons[i]);
      i += 1;
    }
  }
  var n = cpp_construct((node->distance + 1));
  n.parent[MALE] = node;
  node->sons.push_back((&n));
  proc((&n));
  node->sons.pop_back();
}

func withDaughter(node: dynamic, sex: dynamic, proc: dynamic)
{
  {
    var i = 0;
    while ((i < size(node->daughters)))
    {
      proc(node->daughters[i]);
      i += 1;
    }
  }
  var n = cpp_construct((node->distance + 1));
  n.parent[FEMALE] = node;
  node->daughters.push_back((&n));
  proc((&n));
  node->daughters.pop_back();
}

func withBrother(node: dynamic, sex: dynamic, proc: dynamic)
{
  var inner = __cpp_lambda_1;
  withParent(node, sex, bind(withSon, cpp_1, MALE, inner));
}

func withSister(node: dynamic, sex: dynamic, proc: dynamic)
{
  var inner = __cpp_lambda_2;
  withParent(node, sex, bind(withDaughter, cpp_1, MALE, inner));
}

func dfs(node: dynamic, sex: dynamic, index: dynamic)
{
  if ((index == size(relations)))
  {
    ansMax = max(ansMax, node->distance);
    ansMin = min(ansMin, node->distance);
    return;
  }
  var r = relations[index];
  var dfsMale = bind(dfs, cpp_1, MALE, (index + 1));
  var dfsFemale = bind(dfs, cpp_1, FEMALE, (index + 1));
  if ((r == FATHER))
  {
    withParent(node, sex, dfsMale);
  } else if ((r == MOTHER))
  {
    withParent(node, sex, dfsFemale);
  } else if ((r == SON))
  {
    withSon(node, sex, dfsMale);
  } else if ((r == DAUGHTER))
  {
    withDaughter(node, sex, dfsFemale);
  } else if ((r == HUSBAND))
  {
    dfs(node, MALE, (index + 1));
  } else if ((r == WIFE))
  {
    dfs(node, FEMALE, (index + 1));
  } else if ((r == BROTHER))
  {
    withBrother(node, sex, dfsMale);
  } else if ((r == SISTER))
  {
    withSister(node, sex, dfsFemale);
  } else if ((r == GRANDFATHER))
  {
    withParent(node, sex, bind(withParent, cpp_1, MALE, dfsMale));
    withParent(node, sex, bind(withParent, cpp_1, FEMALE, dfsMale));
  } else if ((r == GRANDMOTHER))
  {
    withParent(node, sex, bind(withParent, cpp_1, MALE, dfsFemale));
    withParent(node, sex, bind(withParent, cpp_1, FEMALE, dfsFemale));
  } else if ((r == GRANDSON))
  {
    withSon(node, sex, bind(withSon, cpp_1, MALE, dfsMale));
    withDaughter(node, sex, bind(withSon, cpp_1, FEMALE, dfsMale));
  } else if ((r == GRANDDAUGHTER))
  {
    withSon(node, sex, bind(withDaughter, cpp_1, MALE, dfsFemale));
    withDaughter(node, sex, bind(withDaughter, cpp_1, FEMALE, dfsFemale));
  } else if ((r == UNCLE))
  {
    withParent(node, sex, bind(withBrother, cpp_1, MALE, dfsMale));
    withParent(node, sex, bind(withBrother, cpp_1, FEMALE, dfsMale));
  } else if ((r == AUNT))
  {
    withParent(node, sex, bind(withSister, cpp_1, MALE, dfsFemale));
    withParent(node, sex, bind(withSister, cpp_1, FEMALE, dfsFemale));
  } else if ((r == NEPHEW))
  {
    withBrother(node, sex, bind(withSon, cpp_1, MALE, dfsMale));
    withSister(node, sex, bind(withSon, cpp_1, FEMALE, dfsMale));
  } else if ((r == NIECE))
  {
    withBrother(node, sex, bind(withDaughter, cpp_1, MALE, dfsFemale));
    withSister(node, sex, bind(withDaughter, cpp_1, FEMALE, dfsFemale));
  } else
  {
    assert(false);
  }
}

func main()
{
  var line: dynamic;
  getline(cin, line);
  {
    var T = atoi(line.c_str());
    while (T)
    {
      ansMax = 0;
      ansMin = INT_MAX;
      relations.clear();
      getline(cin, line);
      var words = cpp_construct(split(line, cpp_char(" ")));
      {
        var i = 3;
        while ((i < size(words)))
        {
          if (startsWith(words[i], "father"))
          {
            relations.push_back(FATHER);
          } else if (startsWith(words[i], "mother"))
          {
            relations.push_back(MOTHER);
          } else if (startsWith(words[i], "son"))
          {
            relations.push_back(SON);
          } else if (startsWith(words[i], "daughter"))
          {
            relations.push_back(DAUGHTER);
          } else if (startsWith(words[i], "husband"))
          {
            relations.push_back(HUSBAND);
          } else if (startsWith(words[i], "wife"))
          {
            relations.push_back(WIFE);
          } else if (startsWith(words[i], "brother"))
          {
            relations.push_back(BROTHER);
          } else if (startsWith(words[i], "sister"))
          {
            relations.push_back(SISTER);
          } else if (startsWith(words[i], "grandfather"))
          {
            relations.push_back(GRANDFATHER);
          } else if (startsWith(words[i], "grandmother"))
          {
            relations.push_back(GRANDMOTHER);
          } else if (startsWith(words[i], "grandson"))
          {
            relations.push_back(GRANDSON);
          } else if (startsWith(words[i], "granddaughter"))
          {
            relations.push_back(GRANDDAUGHTER);
          } else if (startsWith(words[i], "uncle"))
          {
            relations.push_back(UNCLE);
          } else if (startsWith(words[i], "aunt"))
          {
            relations.push_back(AUNT);
          } else if (startsWith(words[i], "nephew"))
          {
            relations.push_back(NEPHEW);
          } else if (startsWith(words[i], "niece"))
          {
            relations.push_back(NIECE);
          } else
          {
            assert(false);
          }
          i += 1;
        }
      }
      var root = cpp_construct(0);
      dfs((&root), MALE, 0);
      dfs((&root), FEMALE, 0);
      write(ansMax, " ", ansMin, "\n");
      T -= 1;
    }
  }
}

func __cpp_lambda_1(s: dynamic)
{
  if ((s != node))
  {
    proc(s);
  }
}

func __cpp_lambda_2(d: dynamic)
{
  if ((d != node))
  {
    proc(d);
  }
}
